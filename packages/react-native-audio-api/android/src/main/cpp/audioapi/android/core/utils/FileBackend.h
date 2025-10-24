#pragma once

#include <string>
#include <memory>

namespace audioapi {

enum class AudioFormat;
class AndroidAudioFileOptions;

class FileBackend {
 public:
  explicit FileBackend(std::shared_ptr<AndroidAudioFileOptions> fileOptions)
      : fileOptions_(fileOptions) {}
  virtual ~FileBackend() = default;

  virtual void openFile(std::string filePath, int32_t streamSampleRate, int32_t streamChannelCount) = 0;
  virtual std::string closeFile() = 0;

  virtual bool writeAudioData(void *data, int numFrames) = 0;

 protected:
  std::string filePath_;
  int32_t streamSampleRate_;
  int32_t streamChannelCount_;
  std::shared_ptr<AndroidAudioFileOptions> fileOptions_;
};

} // namespace audioapi
