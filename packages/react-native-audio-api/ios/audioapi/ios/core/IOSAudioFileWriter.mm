#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/IOSAudioFileOptions.h>
#include <audioapi/ios/core/IOSAudioFileWriter.h>

constexpr double BYTES_TO_MB = 1024 * 1024;

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

std::string IOSAudioFileWriter::openFile(AVAudioFormat *bufferFormat)
{
  @autoreleasepool {
    if (audioFile_ != nil) {
      NSLog(@"⚠️ createFileForWriting: currentAudioFile_ already exists");
      return "";
    }

    bufferFormat_ = bufferFormat;

    NSError *error = nil;
    NSDictionary *settings = fileOptions_->getFileSettings();
    fileURL_ = getFileURL();

    NSLog(@"ℹ️ Creating audio file at URL: %@", [fileURL_ absoluteString]);

    audioFile_ = [[AVAudioFile alloc] initForWriting:fileURL_
                                            settings:settings
                                        commonFormat:AVAudioPCMFormatFloat32
                                         interleaved:bufferFormat.interleaved
                                               error:&error];
    converter_ = [[AVAudioConverter alloc] initFromFormat:bufferFormat toFormat:[audioFile_ processingFormat]];
    converter_.sampleRateConverterAlgorithm = AVSampleRateConverterAlgorithm_Normal;
    converter_.sampleRateConverterQuality = AVAudioQualityMax;
    converter_.primeMethod = AVAudioConverterPrimeMethod_None;

    NSLog(@"buferFormat: %@ fileFormat: %@", bufferFormat, [audioFile_ processingFormat]);

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

    return {fileSizeBytesMb, fileDuration};
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

    if (audioBufferList->mNumberBuffers != filePCMFormat.channelCount ||
        bufferFormat_.sampleRate != filePCMFormat.sampleRate ||
        bufferFormat_.interleaved != filePCMFormat.interleaved) {
      int outputFrameCount = ceil(numFrames * filePCMFormat.sampleRate / bufferFormat_.sampleRate);

      AVAudioPCMBuffer *inputBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bufferFormat_
                                                                    frameCapacity:(AVAudioFrameCount)numFrames];

      for (int i = 0; i < bufferFormat_.channelCount; i++) {
        memcpy(
            inputBuffer.mutableAudioBufferList->mBuffers[i].mData,
            audioBufferList->mBuffers[i].mData,
            audioBufferList->mBuffers[i].mDataByteSize);
      }

      inputBuffer.frameLength = numFrames;
      AVAudioPCMBuffer *outputBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:filePCMFormat
                                                                     frameCapacity:outputFrameCount];
      outputBuffer.frameLength = outputFrameCount;

      AVAudioConverterInputBlock inputBlock =
          ^AVAudioBuffer *_Nullable(AVAudioPacketCount inNumberOfPackets, AVAudioConverterInputStatus *outStatus)
      {
        // this line is probably an delusion, but for my sanity lets keep it
        inNumberOfPackets = numFrames;
        *outStatus = AVAudioConverterInputStatus_HaveData;
        return inputBuffer;
      };

      [converter_ convertToBuffer:outputBuffer error:&error withInputFromBlock:inputBlock];

      if (error != nil) {
        NSLog(@"Error during audio conversion: %@", [error debugDescription]);
        return false;
      }

      [audioFile_ writeFromBuffer:outputBuffer error:&error];

      if (error != nil) {
        NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
        return false;
      }
    } else {
      AVAudioPCMBuffer *processingBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:filePCMFormat
                                                                      bufferListNoCopy:audioBufferList
                                                                           deallocator:NULL];

      [audioFile_ writeFromBuffer:processingBuffer error:&error];

      if (error != nil) {
        NSLog(@"Error writing audio data to file: %@", [error debugDescription]);
        return false;
      }
    }

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

} // namespace audioapi
