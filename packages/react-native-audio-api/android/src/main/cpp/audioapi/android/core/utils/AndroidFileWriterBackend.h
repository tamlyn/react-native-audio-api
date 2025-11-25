#pragma once

#include <tuple>
#include <string>
#include <memory>
#include <audioapi/utils/ReturnStatus.hpp>

namespace audioapi {

class AudioFileProperties;

typedef ReturnStatus<std::string> OpenFileStatus;
typedef ReturnStatus<std::tuple<double, double>> CloseFileStatus;

class AndroidFileWriterBackend {
 public:
  explicit AndroidFileWriterBackend(std::shared_ptr<AudioFileProperties> properties) : properties_(properties) {}

  virtual ~AndroidFileWriterBackend() = default;

  virtual OpenFileStatus openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) = 0;
  virtual CloseFileStatus closeFile() = 0;

  virtual bool writeAudioData(void *data, int numFrames) = 0;

  std::string getFilePath() const { return filePath_; }

  double getCurrentDuration() const { return static_cast<double>(framesWritten_.load()) / streamSampleRate_; }

 protected:
  std::shared_ptr<AudioFileProperties> properties_;

  std::string filePath_{""};
  std::atomic<size_t> framesWritten_{0};

  int32_t streamSampleRate_{0};
  int32_t streamChannelCount_{0};
  int32_t streamMaxBufferSize_{0};
};

} // namespace audioapi
