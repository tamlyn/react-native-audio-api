#pragma once

#include <string>
#include <memory>

namespace audioapi {

class AndroidFileWriterBackend {
 public:
  AndroidFileWriterBackend(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags);
  ~AndroidFileWriterBackend() = default;

  virtual void openFile(int32_t streamSampleRate, int32_t streamChannelCount) = 0;
  virtual std::string closeFile() = 0;

  virtual bool writeAudioData(void *data, int numFrames) = 0;

  std::string getFilePath() const { return filePath_; }

 protected:
  std::string filePath_{""};
  int32_t streamSampleRate_;
  int32_t streamChannelCount_;
};

} // namespace audioapi
