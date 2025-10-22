#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/IOSAudioFileWriter.h>

namespace audioapi {
IOSAudioFileWriter::IOSAudioFileWriter(float sampleRate, size_t channelCount, size_t bitRate, size_t iosFlags)
{
  fileOptions_ = std::make_shared<IOSAudioFileOptions>(sampleRate, channelCount, bitRate, iosFlags);
}

IOSAudioFileWriter::~IOSAudioFileWriter()
{
  closeFile();
}

void IOSAudioFileWriter::openFile()
{
  @autoreleasepool {
    if (audioFile_ != nil) {
      NSLog(@"⚠️ createFileForWriting: currentAudioFile_ already exists");
      return;
    }

    NSError *error = nil;
    NSDictionary *settings = fileOptions_->getFileSettings();
    fileURL_ = getFileURL();

    NSLog(@"ℹ️ Creating audio file at URL: %@", [fileURL_ absoluteString]);

    audioFile_ = [[AVAudioFile alloc] initForWriting:fileURL_ settings:settings error:&error];

    if (error != nil || audioFile_ == nil) {
      NSLog(@"Error creating audio file for writing: %@", [error debugDescription]);
      audioFile_ = nil;
    }
  }
}

void IOSAudioFileWriter::closeFile()
{
  @autoreleasepool {
    if (audioFile_ == nil) {
      return;
    }

    // AVAudioFile automatically finalizes the file when deallocated
    audioFile_ = nil;
    fileURL_ = nil;
  }
}

bool IOSAudioFileWriter::writeAudioData(const AudioBufferList *audioBufferList, int numFrames)
{
  if (audioFile_ == nil) {
    NSLog(@"⚠️ writeAudioData: audioFile is nil, cannot write data");
    return false;
  }

  // TODO: not sure if this is necessary
  @autoreleasepool {
    NSError *error = nil;
    AVAudioFormat *filePCMFormat = [audioFile_ processingFormat];
    AVAudioChannelCount fileChannelCount = [filePCMFormat channelCount];

    // Hallucinated check?
    if (!filePCMFormat.isStandard || filePCMFormat.commonFormat != AVAudioPCMFormatFloat32) {
      NSLog(@"⚠️ writeAudioData: Unsupported audio file format for writing");
      return false;
    }

    // Hallucinated check? TODO: verify if writing mixed channel counts is supported
    if (audioBufferList->mNumberBuffers != fileChannelCount) {
      NSLog(@"⚠️ writeAudioData: Mismatched channel count between input buffer and audio file");
      return false;
    }

    AVAudioPCMBuffer *pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:filePCMFormat frameCapacity:numFrames];
    pcmBuffer.frameLength = numFrames;

    for (AVAudioChannelCount channel = 0; channel < fileChannelCount; ++channel) {
      float *dest = pcmBuffer.floatChannelData[channel];
      float *src = (float *)audioBufferList->mBuffers[channel].mData;
      std::memcpy(dest, src, numFrames * sizeof(float));
    }

    [audioFile_ writeFromBuffer:pcmBuffer error:&error];

    if (error != nil) {
      NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
      return false;
    }
  }
}

NSString *IOSAudioFileWriter::getISODateStringForDirectory()
{
  NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
  fmt.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
  fmt.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"]; // or local if you prefer
  fmt.dateFormat = @"yyyy-MM-dd";
  return [fmt stringFromDate:[NSDate date]];
}

NSString *IOSAudioFileWriter::getTimestampForFilename()
{
  NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
  fmt.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
  fmt.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"]; // or local if you prefer
  fmt.dateFormat = @"yyyyMMdd_HHmmss_SSS";
  return [fmt stringFromDate:[NSDate date]];
}

NSURL *getFileURL()
{
  NSError *error = nil;

  NSSearchPathDirectory searchDirectory = fileOptions_->getDirectory();
  NSString *directory = [NSString stringWithFormat:@"AudioAPI/%@", getISODateStringForDirectory()];

  NSURL *baseURL = [[[NSFileManager defaultManager] URLsForDirectory:searchDirectory
                                                           inDomains:NSUserDomainMask] firstObject];
  NSURL *dirURL = [baseURL URLByAppendingPathComponent:directory isDirectory:YES];

  [[NSFileManager defaultManager] createDirectoryAtURL:dirURL
                           withIntermediateDirectories:YES
                                            attributes:nil
                                                 error:&error];

  if (error != nil) {
    NSLog(@"Error creating directory for audio recordings: %@", [error debugDescription]);
    dirURL = baseURL;
  }

  NSString *fileName =
      [NSString stringWithFormat:@"audio_%@.%@", getTimestampForFilename(), fileOptions_->getFileExtension()];
  return [dirURL URLByAppendingPathComponent:fileName];
}

} // namespace audioapi
