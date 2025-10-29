#pragma once

#include <tuple>
#include <string>
#include <memory>

namespace audioapi {

class AndroidFileWriterBackend {
 public:
  AndroidFileWriterBackend(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags) {}

  virtual ~AndroidFileWriterBackend() = default;

  virtual std::string openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) = 0;
  virtual std::tuple<double, double> closeFile() = 0;

  virtual bool writeAudioData(void *data, int numFrames) = 0;

  std::string getFilePath() const { return filePath_; }

  double getCurrentDuration() const { return static_cast<double>(framesWritten_.load()) / streamSampleRate_; }

 protected:
  std::string filePath_{""};
  std::atomic<size_t> framesWritten_{0};

  int32_t streamSampleRate_{0};
  int32_t streamChannelCount_{0};
  int32_t streamMaxBufferSize_{0};
};

} // namespace audioapi
