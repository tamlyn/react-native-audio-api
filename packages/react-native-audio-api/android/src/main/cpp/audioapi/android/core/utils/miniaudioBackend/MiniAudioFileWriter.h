#pragma once

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/libs/miniaudio/miniaudio.h>

#include <string>
#include <memory>

namespace audioapi {

class MiniAudioFileOptions;

class MniAudioFileWriter : public AndroidFileWriterBackend {
 public:
  MniAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags);
  ~MniAudioFileWriter() override;

  void openFile(int32_t streamSampleRate, int32_t streamChannelCount) override;
  std::string closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  std::shared_ptr<MiniAudioFileOptions> fileOptions_;

  int32_t streamSampleRate_{0};
  int32_t streamChannelCount_{0};

  bool isFileOpen_{false};
  bool isConverterRequired_{false};
  std::atomic<bool> isWriting_{false};

  // Miniaudio structures, can they be safely stored as members?
  ma_encoder encoder_{};
  ma_format fileFormat_{};
  ma_data_converter converter_{};
  void *processingBuffer_{nullptr};

  bool initializeConverterIfNeeded();
  bool initializeEncoder();
  ma_uint64 convertBuffer(void *data, int numFrames);

  bool isFileOpen();
};

} // namespace audioapi
