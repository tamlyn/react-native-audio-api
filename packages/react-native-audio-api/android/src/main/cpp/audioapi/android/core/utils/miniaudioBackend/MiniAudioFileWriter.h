#pragma once

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/libs/miniaudio/miniaudio.h>

#include <string>
#include <memory>
#include <tuple>

namespace audioapi {

class MiniAudioFileWriter : public AndroidFileWriterBackend {
 public:
  explicit MiniAudioFileWriter(std::shared_ptr<AudioFileProperties> properties);
  ~MiniAudioFileWriter() override;

  OpenFileStatus openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) override;
  CloseFileStatus closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  std::atomic<bool> isFileOpen_{false};
  std::atomic<bool> isConverterRequired_{false};

  std::unique_ptr<ma_encoder> encoder_{nullptr};
  std::unique_ptr<ma_data_converter> converter_{nullptr};
  void *processingBuffer_{nullptr};
  ma_uint64 processingBufferLength_{0};

  ma_result initializeConverterIfNeeded();
  ma_result initializeEncoder();
  ma_uint64 convertBuffer(void *data, int numFrames);

  bool isFileOpen();
  bool isConverterRequired();
};

} // namespace audioapi
