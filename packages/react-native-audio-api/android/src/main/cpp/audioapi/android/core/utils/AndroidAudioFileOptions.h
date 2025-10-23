#pragma once

#include <cstddef>
#include <cstdint>

namespace audioapi {

class AndroidAudioFileOptions {
 public:
  AndroidAudioFileOptions(float sampleRate, size_t channelCount, size_t bitRate, size_t flags);
  ~AndroidAudioFileOptions() = default;

 private:
  uint8_t format_;
  uint8_t quality_;
  uint8_t flacCompressionLevel_;
  uint8_t directory_;
  uint8_t bitDepth_;
  float sampleRate_;
  size_t channelCount_;
  size_t bitRate_;
};

} // namespace audioapi
