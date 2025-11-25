#pragma once

#include <audioapi/utils/ReturnStatus.hpp>

#include <memory>
#include <string>
#include <tuple>

#ifndef __OBJC__ // when compiled as C++
typedef struct objc_object NSURL;
typedef struct objc_object NSString;
typedef struct objc_object AVAudioFile;
typedef struct objc_object AVAudioFormat;
typedef struct objc_object AudioBufferList;
typedef struct objc_object AVAudioConverter;
#endif // __OBJC__

namespace audioapi {

class AudioFileProperties;
class AudioEventHandlerRegistry;

class FileWriter {
 public:
  FileWriter(
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry,
      const std::shared_ptr<AudioFileProperties> &fileProperties);
  ~FileWriter();

  ReturnStatus<std::string> openFile(AVAudioFormat *bufferFormat, size_t maxInputBufferLength);
  ReturnStatus<std::tuple<double, double>> closeFile();

  bool writeAudioData(const AudioBufferList *audioBufferList, int numFrames);

  double getCurrentDuration() const;
  std::string getFilePath() const;

  void setOnErrorCallback(uint64_t callbackId);
  void clearOnErrorCallback();
  void invokeOnErrorCallback(const std::string &message);

 private:
  size_t converterInputBufferSize_;
  size_t converterOutputBufferSize_;
  std::atomic<size_t> framesWritten_{0};
  std::atomic<uint64_t> errorCallbackId_{0};

  std::shared_ptr<AudioFileProperties> fileProperties_;
  AVAudioFile *audioFile_;
  AVAudioFormat *bufferFormat_;
  AVAudioConverter *converter_;
  NSURL *fileURL_;

  AVAudioPCMBuffer *converterInputBuffer_;
  AVAudioPCMBuffer *converterOutputBuffer_;

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
};

} // namespace audioapi
