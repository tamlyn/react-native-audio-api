#include <android/log.h>
#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/FileOptions.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/libs/miniaudio/miniaudio.h>
#include <audioapi/utils/AudioFileProperties.h>
#include <audioapi/utils/UnitConversion.h>

#include <cstdio>
#include <memory>
#include <string>

namespace audioapi {

/// @brief Get the encoding format based on the audio file properties (only WAV supported).
// Currently, miniaudio supports only WAV encoding, but out of convenience
// or potential future shenanigans, we keep this as a separate function.
inline ma_encoding_format getFormat(const std::shared_ptr<AudioFileProperties> &properties) {
  return ma_encoding_format_wav;
}

/// @brief Get the data format based on the bit depth.
/// @param properties The audio file properties.
/// @return The corresponding ma_format.
inline ma_format getDataFormat(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->bitDepth) {
    case AudioFileProperties::BitDepth::Bit16:
      return ma_format_s16;

    case AudioFileProperties::BitDepth::Bit24:
      return ma_format_s24;

    case AudioFileProperties::BitDepth::Bit32:
      return ma_format_f32;

    default:
      return ma_format_f32;
  }
}

MiniAudioFileWriter::MiniAudioFileWriter(std::shared_ptr<AudioFileProperties> properties)
    : AndroidFileWriterBackend(properties) {}

MiniAudioFileWriter::~MiniAudioFileWriter() {
  isFileOpen_.store(false, std::memory_order_release);
  properties_.reset();

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

/// @brief Opens the audio file for writing.
/// This method initializes the audio converter and encoder together with any
/// necessary buffers required during the writing process.
/// this method should be called only on the JS thread.
/// @param streamSampleRate The sample rate of the incoming audio stream.
/// @param streamChannelCount The channel count of the incoming audio stream.
/// @param streamMaxBufferSize The maximum buffer size of the incoming audio stream.
/// @return The status of the file opening operation.
OpenFileStatus MiniAudioFileWriter::openFile(
    float streamSampleRate,
    int32_t streamChannelCount,
    int32_t streamMaxBufferSize) {
  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  streamMaxBufferSize_ = streamMaxBufferSize;
  ma_result result;

  isConverterRequired_.store(
      (streamSampleRate_ != properties_->sampleRate) ||
      (streamChannelCount_ != properties_->channelCount) ||
      (getDataFormat(properties_) != ma_format_f32));

  result = initializeConverterIfNeeded();

  if (result != MA_SUCCESS) {
    return OpenFileStatus::Error(
        "Failed to initialize converter" + std::string(ma_result_description(result)));
  }

  result = initializeEncoder();

  if (result != MA_SUCCESS) {
    return OpenFileStatus::Error(
        "Failed to initialize encoder" + std::string(ma_result_description(result)));
  }

  isFileOpen_.store(true, std::memory_order_release);
  return OpenFileStatus::Success(filePath_);
}

/// @brief Closes the audio file.
/// This method finalizes the writing process, releases resources,
/// and retrieves the duration and size of the written audio file.
/// It should be called only on the JS thread.
/// @return The status of the file closing operation.
CloseFileStatus MiniAudioFileWriter::closeFile() {
  if (!isFileOpen()) {
    return CloseFileStatus::Error("File is not open");
  }

  isFileOpen_.store(false, std::memory_order_release);

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

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount) == MA_SUCCESS) {
      durationInSeconds = static_cast<double>(frameCount) / decoder.outputSampleRate;
    }

    ma_decoder_uninit(&decoder);
  }

  FILE *file = fopen(filePath_.c_str(), "rb");

  if (file != nullptr) {
    fseek(file, 0, SEEK_END);
    uint64_t fileSizeInBytes = ftell(file);
    fclose(file);
    fileSizeInMB = static_cast<double>(fileSizeInBytes) / MB_IN_BYTES;
  }

  filePath_ = "";
  return CloseFileStatus::Success({fileSizeInMB, durationInSeconds});
}

/// @brief Writes audio data to the file.
/// If possible (sample format, channel count, and interleaving matches),
/// the data is written directly, otherwise in-memory conversion is performed first
/// It should be called only on the audio thread.
/// @param data Pointer to the audio data buffer. (Interleaved float32 format - as oboe likes it)
/// @param numFrames Number of audio frames to write.
/// @return True if the write operation was successful, false otherwise.
bool MiniAudioFileWriter::writeAudioData(void *data, int numFrames) {
  ma_uint64 framesWritten = 0;
  ma_result result;

  if (!isFileOpen()) {
    return false;
  }

  if (!isConverterRequired()) {
    result = ma_encoder_write_pcm_frames(encoder_.get(), data, numFrames, &framesWritten);

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

    framesWritten_.fetch_add(numFrames, std::memory_order_acq_rel);
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

  framesWritten_.fetch_add(numFrames, std::memory_order_acq_rel);
  return result == MA_SUCCESS;
}

/// @brief Converts the audio data buffer if necessary.
/// @param data Pointer to the audio data buffer.
/// @param numFrames Number of audio frames to convert.
/// @return The number of frames after conversion.
ma_uint64 MiniAudioFileWriter::convertBuffer(void *data, int numFrames) {
  ma_uint64 inputFrameCount = numFrames;
  ma_uint64 outputFrameCount = 0;

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), inputFrameCount, &outputFrameCount);

  ma_data_converter_process_pcm_frames(
      converter_.get(), data, &inputFrameCount, processingBuffer_, &outputFrameCount);

  return outputFrameCount;
}

/// @brief Initializes the data converter if needed.
/// This method sets up the data converter and allocates, so it should be called
/// only on the JS thread. (during file opening)
/// @return MA_SUCCESS if initialization was successful, otherwise an error code.
ma_result MiniAudioFileWriter::initializeConverterIfNeeded() {
  if (!isConverterRequired_) {
    return MA_SUCCESS;
  }

  ma_result result;
  ma_format dataFormat = getDataFormat(properties_);

  ma_data_converter_config converterConfig = ma_data_converter_config_init(
      ma_format_f32,
      dataFormat,
      streamChannelCount_,
      properties_->channelCount,
      streamSampleRate_,
      properties_->sampleRate);

  converter_ = std::make_unique<ma_data_converter>();
  result = ma_data_converter_init(&converterConfig, NULL, converter_.get());

  if (result != MA_SUCCESS) {
    return result;
  }

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), streamMaxBufferSize_, &processingBufferLength_);

  processingBuffer_ = ma_malloc(
      processingBufferLength_ * properties_->channelCount * ma_get_bytes_per_sample(dataFormat),
      NULL);

  return MA_SUCCESS;
}

/// @brief Initializes the audio encoder.
/// This method sets up the audio encoder for writing to the file,
/// it should be called only on the JS thread. (during file opening)
/// @return MA_SUCCESS if initialization was successful, otherwise an error code.
ma_result MiniAudioFileWriter::initializeEncoder() {
  ma_result result;
  ResultStatus<std::string> filePathResult = android::fileoptions::getFilePath(properties_);

  if (!filePathResult.isSuccess()) {
    return MA_ERROR;
  }

  filePath_ = filePathResult.getValue();

  ma_encoder_config config = ma_encoder_config_init(
      getFormat(properties_),
      getDataFormat(properties_),
      properties_->channelCount,
      properties_->sampleRate);

  encoder_ = std::make_unique<ma_encoder>();
  result = ma_encoder_init_file(filePath_.c_str(), &config, encoder_.get());

  if (result != MA_SUCCESS) {
    return result;
  }

  return MA_SUCCESS;
}

bool MiniAudioFileWriter::isFileOpen() {
  return isFileOpen_.load(std::memory_order_acquire);
}

bool MiniAudioFileWriter::isConverterRequired() {
  return isConverterRequired_.load(std::memory_order_acquire);
}

} // namespace audioapi
