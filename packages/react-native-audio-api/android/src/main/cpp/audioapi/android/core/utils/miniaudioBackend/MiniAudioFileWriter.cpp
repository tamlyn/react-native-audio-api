#include <android/log.h>
#include <audioapi/libs/miniaudio/miniaudio.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileOptions.h>

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
  fileOptions_.reset();
}

void MiniAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount) {
  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  bool success = false;

  isConverterRequired_ =
      (streamSampleRate_ != fileOptions_->getSampleRate()) ||
      (streamChannelCount_ != fileOptions_->getChannelCount()) ||
      (fileOptions_->getFormat() != ma_format_f32);

  success = initializeConverterIfNeeded();

  if (!success) {
    return;
  }

  success = initializeEncoder();

  if (!success) {
    return;
  }

  isFileOpen_.store(true);
}

std::string MiniAudioFileWriter::closeFile() {
}

bool MiniAudioFileWriter::writeAudioData(void *data, int numFrames) {
  if (!isFileOpen()) {
    return false;
  }

  return true;
}

ma_uint64 MiniAudioFileWriter::convertBuffer(void *data, int numFrames) {
  return 0;
}

bool MiniAudioFileWriter::initializeConverterIfNeeded() {
  if (!isConverterRequired_) {
    return true;
  }

  ma_result result;

  ma_converter_config converterConfig = ma_data_converter_config_init(
    ma_format_f32,
    fileOptions_->getFormat(),
    streamChannelCount_,
    fileOptions_->getChannelCount(),
    streamSampleRate_,
    fileOptions_->getSampleRate()
  );

  result = ma_data_converter_init(&converterConfig, NULL, &converter_);

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniAudioFileWriter",
        "Failed to initialize miniaudio data converter for file: %s",
        filePath_.c_str());
    return false;
  }

  return true;
}

bool MiniAudioFileWriter::initializeEncoder() {
  std::string filePath = fileOptions_->getFilePath(baseFileName_);

  ma_result result;

  ma_encoder_config config = ma_encoder_config_init(
    fileOptions_->getFormat(),
    fileOptions_->getDataFormat(),
    fileOptions_->getChannelCount(),
    fileOptions_->getSampleRate());

  result = ma_encoder_init_file(filePath.c_str(), &config, &encoder_);

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "MiniAudioFileWriter",
        "Failed to initialize miniaudio encoder for file: %s",
        filePath.c_str());
    return false;
  }

  return true;
}

bool MiniAudioFileWriter::isFileOpen() {
  return isFileOpen_.load();
}

} // namespace audioapi
