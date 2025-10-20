#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/core/utils/Constants.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/IOSAudioRecorder.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>
#include <unordered_map>

namespace audioapi {

IOSAudioRecorder::IOSAudioRecorder(
    float sampleRate,
    int bufferLength,
    bool recordToFile,
    const std::string &fileDirectory,
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(sampleRate, bufferLength, recordToFile, fileDirectory, audioEventHandlerRegistry)
{
  currentFileURL_ = nil;
  currentAudioFile_ = nil;

  AudioReceiverBlock audioReceiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
    if (isRunning_.load()) {
      auto *inputChannel = static_cast<float *>(inputBuffer->mBuffers[0].mData);
      writeToBuffers(inputChannel, numFrames);
    }

    while (circularBuffer_->getNumberOfAvailableFrames() >= bufferLength_) {
      auto bus = std::make_shared<AudioBus>(bufferLength_, 1, sampleRate_);
      auto *outputChannel = bus->getChannel(0)->getData();

      circularBuffer_->pop_front(outputChannel, bufferLength_);

      invokeOnAudioReadyCallback(bus, bufferLength_);
    }

    if (recordToFile_) {
      writeToFile(inputBuffer, numFrames);
    }
  };

  audioRecorder_ = [[NativeAudioRecorder alloc] initWithReceiverBlock:audioReceiverBlock
                                                         bufferLength:bufferLength
                                                           sampleRate:sampleRate];
}

IOSAudioRecorder::~IOSAudioRecorder()
{
  stop();
  [audioRecorder_ cleanup];
}

void IOSAudioRecorder::start()
{
  if (isRunning_.load()) {
    return;
  }

  if (recordToFile_) {
    createFileForWriting();
  }

  [audioRecorder_ start];
  isRunning_.store(true);
}

void IOSAudioRecorder::stop()
{
  if (!isRunning_.load()) {
    return;
  }

  isRunning_.store(false);
  [audioRecorder_ stop];

  sendRemainingData();

  if (recordToFile_) {
    releaseFile();
  }
}

void IOSAudioRecorder::writeToFile(const AudioBufferList *inputBuffer, int numFrames)
{
  if (!recordToFile_) {
    return;
  }

  @autoreleasepool {
    if (currentAudioFile_ == nil) {
      return;
    }

    AVAudioFormat *filePCMFormat = [currentAudioFile_ processingFormat];
    const AVAudioChannelCount fileChannelCount = [filePCMFormat channelCount];

    if (!filePCMFormat.isStandard || filePCMFormat.commonFormat != AVAudioPCMFormatFloat32) {
      NSLog(@"⚠️ writeToFile: Unsupported audio file format for writing");
      return;
    }

    if (inputBuffer->mNumberBuffers != fileChannelCount) {
      NSLog(@"⚠️ writeToFile: Mismatched channel count between input buffer and audio file");
      return;
    }

    AVAudioPCMBuffer *pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:filePCMFormat frameCapacity:numFrames];

    pcmBuffer.frameLength = numFrames;

    for (AVAudioChannelCount ch = 0; ch < fileChannelCount; ++ch) {
      float *fileChannelData = pcmBuffer.floatChannelData[ch];
      float *inputChannelData = static_cast<float *>(inputBuffer->mBuffers[ch].mData);

      std::memcpy(fileChannelData, inputChannelData, sizeof(float) * numFrames);
    }

    NSError *error = nil;

    [currentAudioFile_ writeFromBuffer:pcmBuffer error:&error];

    if (error != nil) {
      NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
    }
  }
}

void IOSAudioRecorder::createFileForWriting()
{
  @autoreleasepool {
    if (currentAudioFile_ != nil) {
      NSLog(@"⚠️ createFileForWriting: currentAudioFile_ already exists");
      return;
    }

    NSError *error = nil;

    currentFileURL_ = getFileURL();

    NSLog(@"currentFileURL: %@", currentFileURL_);

    NSDictionary *settings = @{
      AVFormatIDKey : @(kAudioFormatMPEG4AAC),
      AVSampleRateKey : @44100.0,
      AVNumberOfChannelsKey : @1,
      AVEncoderBitRateKey : @128000,
      AVEncoderAudioQualityKey : @(AVAudioQualityHigh)
    };

    currentAudioFile_ = [[AVAudioFile alloc] initForWriting:currentFileURL_ settings:settings error:&error];

    if (error != nil || currentAudioFile_ == nil) {
      NSLog(@"Error creating audio file for writing: %@", [error debugDescription]);
      currentAudioFile_ = nil;
    }
  }
}

void IOSAudioRecorder::releaseFile()
{
  @autoreleasepool {
    currentAudioFile_ = nil;
    currentFileURL_ = nil;
  }
}

static inline NSString *ISODateStringForFolder()
{
  NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
  fmt.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
  fmt.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"]; // or local if you prefer
  fmt.dateFormat = @"yyyy-MM-dd";
  return [fmt stringFromDate:[NSDate date]];
}

static inline NSString *TimestampForFilename()
{
  NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
  fmt.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
  fmt.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"]; // or local if you prefer
  fmt.dateFormat = @"yyyyMMdd_HHmmss_SSS";
  return [fmt stringFromDate:[NSDate date]];
}

NSURL *IOSAudioRecorder::getFileURL()
{
  @autoreleasepool {
    NSSearchPathDirectory dir;

    if (fileDirectory_ == "Document") {
      dir = NSDocumentDirectory;
    } else if (fileDirectory_ == "Cache") {
      dir = NSCachesDirectory;
    } else {
      NSString *tmp = NSTemporaryDirectory();
      NSString *directory = [NSString stringWithFormat:@"AudioAPIRecordings/%@", ISODateStringForFolder()];
      NSString *name = [NSString stringWithFormat:@"recording_%@.m4a", TimestampForFilename()];
      NSURL *base = [NSURL fileURLWithPath:tmp isDirectory:YES];
      NSURL *dirURL = [base URLByAppendingPathComponent:directory isDirectory:YES];

      NSError *error = nil;
      [[NSFileManager defaultManager] createDirectoryAtURL:dirURL
                               withIntermediateDirectories:YES
                                                attributes:nil
                                                     error:&error];

      if (error != nil) {
        NSLog(@"Error creating directory for audio recordings: %@", [error debugDescription]);

        return [base URLByAppendingPathComponent:name];
      }

      return [dirURL URLByAppendingPathComponent:name];
    }

    NSURL *baseURL = [[[NSFileManager defaultManager] URLsForDirectory:dir inDomains:NSUserDomainMask] firstObject];

    NSString *directory = [NSString stringWithFormat:@"AudioAPIRecordings/%@", ISODateStringForFolder()];
    NSURL *dirURL = [baseURL URLByAppendingPathComponent:directory isDirectory:YES];

    NSError *error = nil;
    [[NSFileManager defaultManager] createDirectoryAtURL:dirURL
                             withIntermediateDirectories:YES
                                              attributes:nil
                                                   error:&error];

    if (error != nil) {
      NSLog(@"Error creating directory for audio recordings: %@", [error debugDescription]);
      dirURL = baseURL;
    }

    NSString *name = [NSString stringWithFormat:@"recording_%@.m4a", TimestampForFilename()];

    return [dirURL URLByAppendingPathComponent:name];
  }
}

} // namespace audioapi
