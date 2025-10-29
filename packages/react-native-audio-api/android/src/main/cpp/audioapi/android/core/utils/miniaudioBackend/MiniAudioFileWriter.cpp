#include <android/log.h>
#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileOptions.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/libs/miniaudio/miniaudio.h>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

namespace audioapi {

MiniAudioFileWriter::MiniAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags)
    : AndroidFileWriterBackend(
          sampleRate,
          channelCount,
          bitRate,
          androidFlags) {
  fileOptions_ = std::make_shared<MiniAudioFileOptions>(
      sampleRate, channelCount, bitRate, androidFlags);
}

MiniAudioFileWriter::~MiniAudioFileWriter() {
  isFileOpen_.store(false);
  fileOptions_.reset();

  if (encoder_ != nullptr) {
    ma_encoder_uninit(encoder_.get());
    encoder_.reset();
  }

  if (converter_ != nullptr) {
    ma_data_converter_uninit(converter_.get(), NULL);
    converter_.reset();
  }

  if (processingBuffer_ != nullptr) {
    ma_free(processingBuffer_, NULL);
    processingBuffer_ = nullptr;
    processingBufferLength_ = 0;
  }
}

std::string MiniAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount,
    int32_t streamMaxBufferSize) {
  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  streamMaxBufferSize_ = streamMaxBufferSize;
  bool success = false;

  isConverterRequired_.store(
      (streamSampleRate_ != fileOptions_->getSampleRate()) ||
      (streamChannelCount_ != fileOptions_->getChannelCount()) ||
      (fileOptions_->getDataFormat() != ma_format_f32));

  success = initializeConverterIfNeeded();

  if (!success) {
    return "";
  }

  success = initializeEncoder();

  if (!success) {
    return "";
  }

  isFileOpen_.store(true);

  return filePath_;
}

std::tuple<double, double> MiniAudioFileWriter::closeFile() {
  if (!isFileOpen()) {
    return {0.0, 0.0};
  }

  isFileOpen_.store(false);

  if (encoder_ != nullptr) {
    ma_encoder_uninit(encoder_.get());
    encoder_.reset();
  }

  if (converter_ != nullptr) {
    ma_data_converter_uninit(converter_.get(), NULL);
    converter_.reset();
  }

  if (processingBuffer_ != nullptr) {
    ma_free(processingBuffer_, NULL);
    processingBuffer_ = nullptr;
    processingBufferLength_ = 0;
  }

  // Retrieve duration and file size
  std::string filePath = filePath_;
  double durationInSeconds = 0.0;
  double fileSizeInMB = 0.0;

  ma_decoder decoder;

  if (ma_decoder_init_file(filePath_.c_str(), NULL, &decoder) == MA_SUCCESS) {
    ma_uint64 frameCount = 0;

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount) ==
        MA_SUCCESS) {
      durationInSeconds =
          static_cast<double>(frameCount) / decoder.outputSampleRate;
    }

    ma_decoder_uninit(&decoder);
  }

  FILE *file = fopen(filePath_.c_str(), "rb");

  if (file != nullptr) {
    fseek(file, 0, SEEK_END);
    long fileSizeInBytes = ftell(file);
    fclose(file);
    fileSizeInMB = static_cast<double>(fileSizeInBytes) / BYTES_TO_MB;
  }

  filePath_ = "";
  return {fileSizeInMB, durationInSeconds};
}

bool MiniAudioFileWriter::writeAudioData(void *data, int numFrames) {
  ma_uint64 framesWritten = 0;
  ma_result result;

  if (!isFileOpen()) {
    return false;
  }

  if (!isConverterRequired()) {
    result = ma_encoder_write_pcm_frames(
        encoder_.get(), data, numFrames, &framesWritten);

    __android_log_print(
        ANDROID_LOG_DEBUG,
        "MiniAudioFileWriter",
        "Writing %llu frames without conversion to file: %s",
        framesWritten,
        filePath_.c_str());

    if (result != MA_SUCCESS) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "MiniAudioFileWriter",
          "Failed to write audio data to file: %s",
          filePath_.c_str());
    }

    framesWritten_.fetch_add(numFrames);
    return result == MA_SUCCESS;
  }

  ma_uint64 convertedFrameCount = convertBuffer(data, numFrames);

  result = ma_encoder_write_pcm_frames(
      encoder_.get(), processingBuffer_, convertedFrameCount, &framesWritten);

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniAudioFileWriter",
        "Failed to write converted audio data to file: %s",
        filePath_.c_str());
  }

  framesWritten_.fetch_add(numFrames);
  return result == MA_SUCCESS;
}

ma_uint64 MiniAudioFileWriter::convertBuffer(void *data, int numFrames) {
  ma_uint64 inputFrameCount = numFrames;
  ma_uint64 outputFrameCount = 0;

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), inputFrameCount, &outputFrameCount);

  ma_data_converter_process_pcm_frames(
      converter_.get(),
      data,
      &inputFrameCount,
      processingBuffer_,
      &outputFrameCount);

  return outputFrameCount;
}

bool MiniAudioFileWriter::initializeConverterIfNeeded() {
  if (!isConverterRequired_) {
    return true;
  }

  ma_result result;

  ma_data_converter_config converterConfig = ma_data_converter_config_init(
      ma_format_f32,
      fileOptions_->getDataFormat(),
      streamChannelCount_,
      fileOptions_->getChannelCount(),
      streamSampleRate_,
      fileOptions_->getSampleRate());

  converter_ = std::make_unique<ma_data_converter>();
  result = ma_data_converter_init(&converterConfig, NULL, converter_.get());

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniAudioFileWriter",
        "Failed to initialize miniaudio data converter for file: %s",
        filePath_.c_str());
    return false;
  }

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), streamMaxBufferSize_, &processingBufferLength_);

  processingBuffer_ = ma_malloc(
      processingBufferLength_ * fileOptions_->getChannelCount() *
          ma_get_bytes_per_sample(fileOptions_->getDataFormat()),
      NULL);

  return true;
}

bool MiniAudioFileWriter::initializeEncoder() {
  filePath_ = fileOptions_->getFilePath("audio");

  ma_result result;

  ma_encoder_config config = ma_encoder_config_init(
      fileOptions_->getFormat(),
      fileOptions_->getDataFormat(),
      fileOptions_->getChannelCount(),
      fileOptions_->getSampleRate());

  encoder_ = std::make_unique<ma_encoder>();
  result = ma_encoder_init_file(filePath_.c_str(), &config, encoder_.get());

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniAudioFileWriter",
        "Failed to initialize miniaudio encoder for file: %s",
        filePath_.c_str());
    return false;
  }

  return true;
}

bool MiniAudioFileWriter::isFileOpen() {
  return isFileOpen_.load();
}

bool MiniAudioFileWriter::isConverterRequired() {
  return isConverterRequired_.load();
}

} // namespace audioapi
