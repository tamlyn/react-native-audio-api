#pragma once

#include <memory>
#include <string>

namespace audioapi {

class AndroidAudioFileOptions;

class AndroidAudioFileWriter {
 public:
  AndroidAudioFileWriter(float sampleRate, size_t channelCount, size_t bitRate, size_t androidFlags);
  ~AndroidAudioFileWriter();

  void openFile();
  std::string closeFile();

  bool writeAudioData(const int16_t *audioData, int numFrames);

 private:
  std::shared_ptr<AndroidAudioFileOptions> fileOptions_;
};

} // namespace audioapi
