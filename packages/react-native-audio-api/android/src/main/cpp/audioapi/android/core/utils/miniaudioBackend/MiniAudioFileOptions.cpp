
#include <audioapi/android/core/utils/FileUtils.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileOptions.h>

namespace audioapi {

MiniAudioFileOptions::MiniAudioFileOptions(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t flags) {
  sampleRate_ = sampleRate;
  channelCount_ = channelCount;
  bitRate_ = bitRate;

  format_ = static_cast<uint8_t>(flags & 0xF);
  directory_ = static_cast<uint8_t>((flags >> 4) & 0xF);
  bitDepth_ = static_cast<uint8_t>((flags >> 8) & 0xF);
};

ma_encoding_format MiniAudioFileOptions::getFormat() const {
  return ma_encoding_format_wav;
}

std::string MiniAudioFileOptions::getFilePath(
    const std::string &baseFileName) const {
  android::fileutils::FileDirectory dirEnum =
      android::fileutils::directoryFromFlag(directory_);

  return android::fileutils::getFilePath(dirEnum, baseFileName, "wav");
}

float MiniAudioFileOptions::getSampleRate() const {
  return sampleRate_;
}

size_t MiniAudioFileOptions::getChannelCount() const {
  return channelCount_;
}

ma_format MiniAudioFileOptions::getDataFormat() const {
  switch (bitDepth_) {
    case 1:
      return ma_format_s16;
    case 2:
      return ma_format_s24;
    case 3:
      return ma_format_f32;
    default:
      return ma_format_f32;
  }
}

} // namespace audioapi
