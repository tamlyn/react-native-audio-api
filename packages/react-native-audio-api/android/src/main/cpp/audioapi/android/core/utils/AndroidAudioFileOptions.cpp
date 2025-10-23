
#include <audioapi/android/core/utils/AndroidAudioFileOptions.h>

namespace audioapi {

AndroidAudioFileOptions::AndroidAudioFileOptions(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t flags) {
  sampleRate_ = sampleRate;
  channelCount_ = channelCount;
  bitRate_ = bitRate;

  format_ = static_cast<uint8_t>(flags & 0xFF);
  quality_ = static_cast<uint8_t>((flags >> 8) & 0xFF);
  flacCompressionLevel_ = static_cast<uint8_t>((flags >> 16) & 0xFF);
  directory_ = static_cast<uint8_t>((flags >> 24) & 0xFF);
  bitDepth_ = static_cast<uint8_t>((flags >> 32) & 0xFF);
}

} // namespace audioapi
