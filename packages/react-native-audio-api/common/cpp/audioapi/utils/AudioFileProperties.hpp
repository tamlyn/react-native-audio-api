#pragma once

#include <audioapi/jsi/JsiHostObject.h>

#include <algorithm>

namespace audioapi {

struct AudioFileProperties {
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

  static std::shared_ptr<AudioFileProperties> CreateFromJSIValue(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::Value &value) {
    auto options = value.getObject(runtime);

    FileDirectory directory = static_cast<FileDirectory>(
        options.getProperty(runtime, "directory").getNumber());

    std::string subDirectory = options
        .getProperty(runtime, "subDirectory")
        .asString(runtime)
        .utf8(runtime);

    std::string fileNamePrefix = options
        .getProperty(runtime, "fileNamePrefix")
        .asString(runtime)
        .utf8(runtime);

    size_t channelCount = static_cast<size_t>(
        options.getProperty(runtime, "channelCount").getNumber());

    size_t batchDurationSeconds = static_cast<size_t>(
        options.getProperty(runtime, "batchDurationSeconds").getNumber());

    Format format = static_cast<Format>(
        options.getProperty(runtime, "format").getNumber());

    auto presetOptions = options.getProperty(runtime, "preset").getObject(runtime);

    size_t sampleRate = static_cast<size_t>(
        presetOptions.getProperty(runtime, "sampleRate").getNumber());

    size_t bitRate = static_cast<size_t>(
        presetOptions.getProperty(runtime, "bitRate").getNumber());

    BitDepth bitDepth = static_cast<BitDepth>(
        presetOptions.getProperty(runtime, "bitDepth").getNumber());

    int flacCompressionLevel = static_cast<int>(
        presetOptions.getProperty(runtime, "flacCompressionLevel").getNumber());

    IOSAudioQuality iosAudioQuality = static_cast<IOSAudioQuality>(
        presetOptions.getProperty(runtime, "iosQuality").getNumber());

    return std::make_shared<AudioFileProperties>(
      directory,
      subDirectory,
      fileNamePrefix,
      channelCount,
      batchDurationSeconds,
      format,
      sampleRate,
      bitRate,
      bitDepth,
      std::max(flacCompressionLevel - 1, 0),
      iosAudioQuality);
  }

  AudioFileProperties(
    FileDirectory directory,
    const std::string &subDirectory,
    const std::string &fileNamePrefix,
    size_t channelCount,
    size_t batchDurationSeconds,
    Format format,
    size_t sampleRate,
    size_t bitRate,
    BitDepth bitDepth,
    int flacCompressionLevel,
    IOSAudioQuality iosAudioQuality)
    : directory(directory),
      subDirectory(subDirectory),
      fileNamePrefix(fileNamePrefix),
      channelCount(channelCount),
      batchDurationSeconds(batchDurationSeconds),
      format(format),
      sampleRate(sampleRate),
      bitRate(bitRate),
      bitDepth(bitDepth),
      flacCompressionLevel(flacCompressionLevel),
      iosAudioQuality(iosAudioQuality) {}

  FileDirectory directory;
  std::string subDirectory;
  std::string fileNamePrefix;
  size_t channelCount;
  size_t batchDurationSeconds;
  Format format;
  size_t sampleRate;
  size_t bitRate;
  BitDepth bitDepth;
  int flacCompressionLevel;
  IOSAudioQuality iosAudioQuality;
};

} // namespace audioapi
