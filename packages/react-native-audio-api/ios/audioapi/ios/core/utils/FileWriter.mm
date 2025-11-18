#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/utils/FileOptions.h>
#include <audioapi/ios/core/utils/FileWriter.h>
#include <audioapi/utils/AudioFileProperties.hpp>
#include <audioapi/utils/ReturnStatus.hpp>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

namespace audioapi {
FileWriter::FileWriter(const std::shared_ptr<AudioFileProperties> &fileProperties) : fileProperties_(fileProperties) {}

FileWriter::~FileWriter()
{
  fileURL_ = nil;
  audioFile_ = nil;
  converter_ = nil;
  bufferFormat_ = nil;
}

ReturnStatus<std::string> FileWriter::openFile(AVAudioFormat *bufferFormat, size_t maxInputBufferLength)
{
  @autoreleasepool {
    if (audioFile_ != nil) {
      return ReturnStatus<std::string>::Error("file already open");
    }

    framesWritten_.store(0);
    bufferFormat_ = bufferFormat;

    NSError *error = nil;
    NSDictionary *settings = fileoptions::getFileSettings(fileProperties_);
    fileURL_ = fileoptions::getFileURL(fileProperties_);

    if (fileProperties_->sampleRate == 0 || fileProperties_->channelCount == 0) {
      return ReturnStatus<std::string>::Error(
          "Invalid file properties: sampleRate and channelCount must be greater than 0");
    }

    if (bufferFormat.sampleRate == 0 || bufferFormat.channelCount == 0) {
      return ReturnStatus<std::string>::Error(
          "Invalid input format: sampleRate and channelCount must be greater than 0");
    }

    audioFile_ = [[AVAudioFile alloc] initForWriting:fileURL_
                                            settings:settings
                                        commonFormat:AVAudioPCMFormatFloat32
                                         interleaved:bufferFormat.interleaved
                                               error:&error];

    if (error != nil) {
      return ReturnStatus<std::string>::Error(
          std::string("Error creating audio file for writing: ") + [[error debugDescription] UTF8String]);
    }

    converter_ = [[AVAudioConverter alloc] initFromFormat:bufferFormat toFormat:[audioFile_ processingFormat]];
    converter_.sampleRateConverterAlgorithm = AVSampleRateConverterAlgorithm_Normal;
    converter_.sampleRateConverterQuality = AVAudioQualityMax;
    converter_.primeMethod = AVAudioConverterPrimeMethod_None;

    converterInputBufferSize_ = maxInputBufferLength;
    converterOutputBufferSize_ = std::max(
        (double)maxInputBufferLength, fileProperties_->sampleRate / bufferFormat.sampleRate * maxInputBufferLength);

    converterInputBuffer_ = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bufferFormat
                                                          frameCapacity:(AVAudioFrameCount)maxInputBufferLength];
    converterOutputBuffer_ = [[AVAudioPCMBuffer alloc] initWithPCMFormat:[audioFile_ processingFormat]
                                                           frameCapacity:(AVAudioFrameCount)converterOutputBufferSize_];

    if (converterInputBuffer_ == nil || converterOutputBuffer_ == nil || audioFile_ == nil || converter_ == nil) {
      audioFile_ = nil;
      converter_ = nil;
      converterInputBuffer_ = nil;
      converterOutputBuffer_ = nil;

      return ReturnStatus<std::string>::Error("Error creating converter buffers");
    }

    return ReturnStatus<std::string>::Success([[fileURL_ path] UTF8String]);
  }
}

ReturnStatus<std::tuple<double, double>> FileWriter::closeFile()
{
  @autoreleasepool {
    NSError *error;
    std::string filePath = [[fileURL_ path] UTF8String];

    if (audioFile_ == nil) {
      return ReturnStatus<std::tuple<double, double>>::Error("file is not open: " + filePath);
    }

    // AVAudioFile automatically finalizes the file when deallocated
    audioFile_ = nil;

    double fileDuration = CMTimeGetSeconds([[AVURLAsset URLAssetWithURL:fileURL_ options:nil] duration]);
    double fileSizeBytesMb =
        static_cast<double>([[[NSFileManager defaultManager] attributesOfItemAtPath:fileURL_.path
                                                                              error:&error] fileSize]) /
        BYTES_TO_MB;

    if (error != nil) {
      NSLog(@"⚠️ closeFile: error while retrieving file size");
      fileSizeBytesMb = 0;
    }

    fileURL_ = nil;
    framesWritten_.store(0);

    return ReturnStatus<std::tuple<double, double>>::Success(std::make_tuple(fileDuration, fileSizeBytesMb));
  }
}

bool FileWriter::writeAudioData(const AudioBufferList *audioBufferList, int numFrames)
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
    converterOutputBuffer_.frameLength = fileProperties_->sampleRate / bufferFormat_.sampleRate * numFrames;

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

double FileWriter::getCurrentDuration() const
{
  return static_cast<double>(framesWritten_.load()) / bufferFormat_.sampleRate;
}

std::string FileWriter::getFilePath() const
{
  return [[fileURL_ path] UTF8String];
}

} // namespace audioapi
