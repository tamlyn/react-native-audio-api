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
  float sampleRate_;
  size_t channelCount_;
  size_t flags_;
};

} // namespace audioapi
