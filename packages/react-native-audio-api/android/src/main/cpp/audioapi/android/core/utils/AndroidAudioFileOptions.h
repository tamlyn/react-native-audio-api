#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace audioapi {

enum class AudioFormat {
  WAV = 1,
  CAF = 2,
  M4A = 3,
  FLAC = 4,
};

class AndroidAudioFileOptions {
 public:
  AndroidAudioFileOptions(float sampleRate, size_t channelCount, size_t bitRate, size_t flags);
  ~AndroidAudioFileOptions() = default;

  AudioFormat getFormat() const;
  std::string getDirectory() const;
  uint32_t getBitDepth() const;
  std::string getFileExtension() const;

  float getSampleRate() const;
  size_t getChannelCount() const;
  size_t getBitRate() const;

  std::string getFilePath(const std::string &baseFileName) const;

 private:
  uint8_t format_;
  uint8_t directory_;
  uint8_t bitDepth_;
  float sampleRate_;
  size_t channelCount_;
  size_t bitRate_;

  std::string getISODateStringForDirectory() const;
  std::string getTimestampForFileName() const;
  bool ensureDirectoryExists(std::string &path) const;
};

} // namespace audioapi
