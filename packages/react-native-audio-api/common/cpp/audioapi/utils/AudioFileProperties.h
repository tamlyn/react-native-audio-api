#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace facebook {
namespace jsi {
class Runtime;
class Value;
} // namespace jsi
} // namespace facebook

namespace audioapi {

class AudioFileProperties {
 public:
  enum class Format {
    WAV = 1,
    CAF = 2,
    M4A = 3,
    FLAC = 4,
  };

  enum class BitDepth {
    Bit16 = 1,
    Bit24 = 2,
    Bit32 = 3,
  };

  enum class IOSAudioQuality {
    Min = 1,
    Low = 2,
    Medium = 3,
    High = 4,
    Max = 5,
  };

  enum class FileDirectory {
    Document = 1,
    Cache = 2,
  };

  AudioFileProperties(
      FileDirectory directory,
      const std::string &subDirectory,
      const std::string &fileNamePrefix,
      int channelCount,
      size_t batchDurationSeconds,
      Format format,
      float sampleRate,
      size_t bitRate,
      BitDepth bitDepth,
      int flacCompressionLevel,
      int androidFlushIntervalMs,
      IOSAudioQuality iosAudioQuality);

  static std::shared_ptr<AudioFileProperties> CreateFromJSIValue(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::Value &value);

  FileDirectory directory;
  std::string subDirectory;
  std::string fileNamePrefix;
  int channelCount;
  size_t batchDurationSeconds;
  Format format;
  float sampleRate;
  size_t bitRate;
  BitDepth bitDepth;
  int flacCompressionLevel;
  int androidFlushIntervalMs;
  IOSAudioQuality iosAudioQuality;
};

} // namespace audioapi
