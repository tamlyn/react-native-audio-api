#include <android/log.h>
#include <audioapi/android/core/utils/AndroidAudioFileOptions.h>
#include <audioapi/android/core/utils/MiniaudioFileBackend.h>
#include <audioapi/libs/miniaudio/miniaudio.h>

namespace audioapi {

void MiniaudioFileBackend::openFile(
    std::string filePath,
    int32_t streamSampleRate,
    int32_t streamChannelCount) {
  converterConfig_ = ma_data_converter_config_init(
      ma_format_f32,
      fileFormat_,
      streamChannelCount,
      fileOptions_->getChannelCount(),
      streamSampleRate,
      fileOptions_->getSampleRate());

  result = ma_data_converter_init(&converterConfig_, NULL, &converter_);
  ma_uint64 outputFrameCap;
  ma_data_converter_get_expected_output_frame_count(
      &converter_, 2048, &outputFrameCap);
  converterOutput_ = ma_malloc(
      outputFrameCap * fileOptions_->getChannelCount() *
          ma_get_bytes_per_sample(fileFormat_),
      NULL);

  encoderConfig_ = ma_encoder_config_init(
      ma_encoding_format_wav,
      fileFormat_,
      fileOptions_->getChannelCount(),
      fileOptions_->getSampleRate());

  result = ma_encoder_init_file(filePath_.c_str(), &encoderConfig_, &encoder_);

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniaudioFileBackend",
        "Failed to initialize miniaudio encoder for file: %s",
        filePath_.c_str());
    return;
  }
}

std::string MiniaudioFileBackend::closeFile() {
  ma_encoder_uninit(&encoder_);
  ma_data_converter_uninit(&converter_, NULL);
  encoder_ = {};
  encoderConfig_ = {};
  ma_free(converterOutput_, NULL);
  converterOutput_ = nullptr;
  return filePath_;
}

bool MiniaudioFileBackend::writeAudioData(void *data, int numFrames) {
  ma_uint64 framesWritten;

  if (streamSampleRate_ != fileOptions_->getSampleRate() ||
      streamChannelCount_ != fileOptions_->getChannelCount()) {
    ma_uint64 outputFrameCount = convertBuffer(data, numFrames);

    ma_result result = ma_encoder_write_pcm_frames(
        &encoder_, converterOutput_, outputFrameCount, &framesWritten);

    if (result != MA_SUCCESS) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "MiniaudioFileBackend",
          "Failed to write converted audio data to file: %s",
          filePath_.c_str());
      return false;
    }

    return true;
  }

  ma_result result =
      ma_encoder_write_pcm_frames(&encoder_, data, numFrames, &framesWritten);

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniaudioFileBackend",
        "Failed to write audio data to file: %s",
        filePath_.c_str());
    return false;
  }

  return true;
}

ma_uint64 MiniaudioFileBackend::convertBuffer(void *data, int numFrames) {
  ma_uint64 inputFrameCount = (ma_uint64)numFrames;
  ma_uint64 outputFrameCount;
  ma_data_converter_get_expected_output_frame_count(
      &converter_, inputFrameCount, &outputFrameCount);

  ma_uint64 bytesPerFrame =
      fileOptions_->getChannelCount() * ma_get_bytes_per_sample(fileFormat_);

  ma_data_converter_process_pcm_frames(
      &converter_, data, &inputFrameCount, converterOutput_, &outputFrameCount);

  return outputFrameCount;
}

} // namespace audioapi
