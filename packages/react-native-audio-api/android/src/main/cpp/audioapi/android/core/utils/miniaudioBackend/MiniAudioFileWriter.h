#pragma once

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/libs/miniaudio/miniaudio.h>

#include <string>
#include <memory>
#include <tuple>

namespace audioapi {

class MiniAudioFileOptions;

class MiniAudioFileWriter : public AndroidFileWriterBackend {
 public:
  MiniAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags);
  ~MiniAudioFileWriter() override;

  std::string openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) override;
  std::tuple<double, double> closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  std::shared_ptr<MiniAudioFileOptions> fileOptions_;

  std::atomic<bool> isFileOpen_{false};
  std::atomic<bool> isConverterRequired_{false};

  std::unique_ptr<ma_encoder> encoder_{nullptr};
  std::unique_ptr<ma_data_converter> converter_{nullptr};
  void *processingBuffer_{nullptr};
  ma_uint64 processingBufferLength_{0};

  bool initializeConverterIfNeeded();
  bool initializeEncoder();
  ma_uint64 convertBuffer(void *data, int numFrames);

  bool isFileOpen();
  bool isConverterRequired();
};

} // namespace audioapi
