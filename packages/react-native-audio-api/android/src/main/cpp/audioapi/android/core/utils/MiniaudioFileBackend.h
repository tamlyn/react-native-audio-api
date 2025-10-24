#pragma once

#include <audioapi/android/core/utils/FileBackend.h>
#include <audioapi/libs/miniaudio/miniaudio.h>
#include <string>
#include <memory>

namespace audioapi {

enum class AudioFormat;
class AndroidAudioFileOptions;

class MiniaudioFileBackend : public FileBackend {
 public:
  explicit MiniaudioFileBackend(std::shared_ptr<AndroidAudioFileOptions> fileOptions)
      : FileBackend(fileOptions) {}
  ~MiniaudioFileBackend() override = default;

  void openFile(std::string filePath, int32_t streamSampleRate, int32_t streamChannelCount) override;
  std::string closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  // internal state for miniaudio
  ma_encoder_config encoderConfig_{};
  ma_encoder encoder_{};
  ma_data_converter_config converterConfig_{};
  ma_data_converter converter_{};
  ma_format fileFormat_{};
  int32_t streamSampleRate_{0};
  int32_t streamChannelCount_{0};
  void *converterOutput_{nullptr};

  ma_uint64 convertBuffer(void *data, int numFrames);
};

} // namespace audioapi
