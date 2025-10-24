#pragma once

#include <memory>
#include <string>
#include <vector>

namespace audioapi {

class FileBackend;
class AndroidAudioFileOptions;

class AndroidAudioFileWriter {
 public:
  AndroidAudioFileWriter(float sampleRate, size_t channelCount, size_t bitRate, size_t androidFlags);
  ~AndroidAudioFileWriter();

  void openFile(int32_t streamSampleRate, int32_t streamChannelCount);
  std::string closeFile();

  bool writeAudioData(void *data, int numFrames);

 private:
  std::shared_ptr<FileBackend> fileBackend_;
  std::shared_ptr<AndroidAudioFileOptions> fileOptions_;
};

} // namespace audioapi
