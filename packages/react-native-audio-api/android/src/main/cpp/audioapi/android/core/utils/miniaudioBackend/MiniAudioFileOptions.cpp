
#include <audioapi/android/core/utils/FileUtils.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileOptions.h>

namespace audioapi {

MiniAudioFileOptions::MiniAudioFileOptions(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t flags)
    : sampleRate_(sampleRate), channelCount_(channelCount), flags_(flags) {};

ma_encoding_format MiniAudioFileOptions::getFormat() const {
  return ma_encoding_format_wav;
}

std::string MiniAudioFileOptions::getFilePath(
    const std::string &baseFileName) const {
  return android::fileutils::getFilePath(
      android::fileutils::directoryFromFlags(flags_), baseFileName, "wav");
}

float MiniAudioFileOptions::getSampleRate() const {
  return sampleRate_;
}

size_t MiniAudioFileOptions::getChannelCount() const {
  return channelCount_;
}

ma_format MiniAudioFileOptions::getDataFormat() const {
  android::fileutils::BitDepth bitDepth =
      android::fileutils::bitDepthFromFlags(flags_);
  switch (bitDepth) {
    case android::fileutils::BitDepth::BIT_16:
      return ma_format_s16;
    case android::fileutils::BitDepth::BIT_24:
      return ma_format_s24;
    case android::fileutils::BitDepth::BIT_32:
      return ma_format_f32;
    default:
      return ma_format_f32;
  }
}

} // namespace audioapi
