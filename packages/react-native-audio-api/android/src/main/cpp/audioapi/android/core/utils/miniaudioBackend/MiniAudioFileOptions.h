#pragma once

#include <audioapi/libs/miniaudio/miniaudio.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace audioapi {

class MiniAudioFileOptions {
 public:
  MiniAudioFileOptions(float sampleRate, size_t channelCount, size_t bitRate, size_t flags);
  ~MiniAudioFileOptions() = default;

  ma_encoding_format getFormat() const;
  std::string getFilePath(const std::string &baseFileName) const;

  float getSampleRate() const;
  size_t getChannelCount() const;
  ma_format getDataFormat() const;

 private:
  uint8_t format_;
  uint8_t bitDepth_;
  uint8_t directory_;
  float sampleRate_;
  size_t channelCount_;

  // We store the bitRate just that we can
  // but with miniaudio limit to WAV files only
  // we don't really need it here
  size_t bitRate_;
};

} // namespace audioapi
