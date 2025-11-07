#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/IOSAudioFileOptions.h>
#include <audioapi/ios/core/IOSAudioFileWriter.h>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

namespace audioapi {
IOSAudioFileWriter::IOSAudioFileWriter(float sampleRate, size_t channelCount, size_t bitRate, size_t iosFlags)
{
  fileOptions_ = std::make_shared<IOSAudioFileOptions>(sampleRate, channelCount, bitRate, iosFlags);
}

IOSAudioFileWriter::~IOSAudioFileWriter()
{
  fileURL_ = nil;
  audioFile_ = nil;
  converter_ = nil;
  bufferFormat_ = nil;
}

std::string IOSAudioFileWriter::openFile(AVAudioFormat *bufferFormat, size_t maxInputBufferLength)
{
  @autoreleasepool {
    if (audioFile_ != nil) {
      NSLog(@"⚠️ createFileForWriting: currentAudioFile_ already exists");
      return "";
    }

    framesWritten_.store(0);

    bufferFormat_ = bufferFormat;

    NSError *error = nil;
    NSDictionary *settings = fileOptions_->getFileSettings();
    fileURL_ = getFileURL();

    audioFile_ = [[AVAudioFile alloc] initForWriting:fileURL_
                                            settings:settings
                                        commonFormat:AVAudioPCMFormatFloat32
                                         interleaved:bufferFormat.interleaved
                                               error:&error];
    converter_ = [[AVAudioConverter alloc] initFromFormat:bufferFormat toFormat:[audioFile_ processingFormat]];
    converter_.sampleRateConverterAlgorithm = AVSampleRateConverterAlgorithm_Normal;
    converter_.sampleRateConverterQuality = AVAudioQualityMax;
    converter_.primeMethod = AVAudioConverterPrimeMethod_None;

    converterInputBufferSize_ = maxInputBufferLength;
    converterOutputBufferSize_ = std::max(
        (double)maxInputBufferLength, fileOptions_->getSampleRate() / bufferFormat.sampleRate * maxInputBufferLength);

    converterInputBuffer_ = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bufferFormat
                                                          frameCapacity:(AVAudioFrameCount)maxInputBufferLength];
    converterOutputBuffer_ = [[AVAudioPCMBuffer alloc] initWithPCMFormat:[audioFile_ processingFormat]
                                                           frameCapacity:(AVAudioFrameCount)converterOutputBufferSize_];

    if (error != nil || audioFile_ == nil) {
      NSLog(@"Error creating audio file for writing: %@", [error debugDescription]);
      audioFile_ = nil;

      return "";
    }

    return [[fileURL_ path] UTF8String];
  }
}

std::tuple<double, double> IOSAudioFileWriter::closeFile()
{
  @autoreleasepool {
    NSError *error;
    std::string filePath = [[fileURL_ path] UTF8String];

    if (audioFile_ == nil) {
      return {0, 0};
    }

    // AVAudioFile automatically finalizes the file when deallocated
    audioFile_ = nil;

    double fileDuration = CMTimeGetSeconds([[AVURLAsset URLAssetWithURL:fileURL_ options:nil] duration]);
    double fileSizeBytesMb =
        static_cast<double>([[[NSFileManager defaultManager] attributesOfItemAtPath:fileURL_.path
                                                                              error:&error] fileSize]) /
        BYTES_TO_MB;

    NSLog(
        @"ℹ️ Closed audio file at path: %s, duration: %.2f sec, size: %.2f MB",
        filePath.c_str(),
        fileDuration,
        fileSizeBytesMb);

    if (error != nil) {
      NSLog(@"⚠️ closeFile: error while retrieving file size");
      fileSizeBytesMb = 0;
    }

    fileURL_ = nil;
    framesWritten_.store(0);

    return {fileSizeBytesMb, fileDuration};
  }
}

bool IOSAudioFileWriter::writeAudioData(const AudioBufferList *audioBufferList, int numFrames)
{
  if (audioFile_ == nil) {
    NSLog(@"⚠️ writeAudioData: audioFile is nil, cannot write data");
    return false;
  }

  @autoreleasepool {
    NSError *error = nil;
    AVAudioFormat *fileFormat = [audioFile_ processingFormat];

    if (bufferFormat_.sampleRate == fileFormat.sampleRate && bufferFormat_.channelCount == fileFormat.channelCount &&
        bufferFormat_.isInterleaved == fileFormat.isInterleaved) {
      // We can use the converter input buffer as a "transport" layer to the file
      for (size_t i = 0; i < bufferFormat_.channelCount; ++i) {
        memcpy(
            converterInputBuffer_.mutableAudioBufferList->mBuffers[i].mData,
            audioBufferList->mBuffers[i].mData,
            audioBufferList->mBuffers[i].mDataByteSize);
      }
      converterInputBuffer_.frameLength = numFrames;

      [audioFile_ writeFromBuffer:converterInputBuffer_ error:&error];

      if (error != nil) {
        NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
        return false;
      }

      framesWritten_.fetch_add(numFrames);
      return true;
    }

    size_t outputFrameCount = ceil(numFrames * fileFormat.sampleRate / bufferFormat_.sampleRate);

    for (size_t i = 0; i < bufferFormat_.channelCount; ++i) {
      memcpy(
          converterInputBuffer_.mutableAudioBufferList->mBuffers[i].mData,
          audioBufferList->mBuffers[i].mData,
          audioBufferList->mBuffers[i].mDataByteSize);
    }

    converterInputBuffer_.frameLength = numFrames;

    __block BOOL handedOff = false;
    AVAudioConverterInputBlock inputBlock =
        ^AVAudioBuffer *_Nullable(AVAudioPacketCount inNumberOfPackets, AVAudioConverterInputStatus *outStatus)
    {
      if (handedOff) {
        *outStatus = AVAudioConverterInputStatus_NoDataNow;
        return nil;
      }

      handedOff = true;
      *outStatus = AVAudioConverterInputStatus_HaveData;
      return converterInputBuffer_;
    };

    [converter_ convertToBuffer:converterOutputBuffer_ error:&error withInputFromBlock:inputBlock];
    converterOutputBuffer_.frameLength = fileOptions_->getSampleRate() / bufferFormat_.sampleRate * numFrames;

    if (error != nil) {
      NSLog(@"Error during audio conversion: %@", [error debugDescription]);
      return false;
    }

    [audioFile_ writeFromBuffer:converterOutputBuffer_ error:&error];

    if (error != nil) {
      NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
      return false;
    }

    framesWritten_.fetch_add(numFrames);
    return true;
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

NSURL *IOSAudioFileWriter::getFileURL()
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

double IOSAudioFileWriter::getCurrentDuration() const
{
  return static_cast<double>(framesWritten_.load()) / bufferFormat_.sampleRate;
}

} // namespace audioapi
