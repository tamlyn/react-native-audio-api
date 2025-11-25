#include <audioapi/utils/AudioFileProperties.h>

#include <audioapi/jsi/JsiHostObject.h>
#include <jsi/jsi.h>
#include <algorithm>

namespace audioapi {

AudioFileProperties::AudioFileProperties(
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

std::shared_ptr<AudioFileProperties> AudioFileProperties::CreateFromJSIValue(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  auto options = value.getObject(runtime);

  FileDirectory directory = static_cast<FileDirectory>(
      options.getProperty(runtime, "directory").getNumber());

  std::string subDirectory = options.getProperty(runtime, "subDirectory")
                                 .asString(runtime)
                                 .utf8(runtime);

  std::string fileNamePrefix = options.getProperty(runtime, "fileNamePrefix")
                                   .asString(runtime)
                                   .utf8(runtime);

  size_t channelCount = static_cast<size_t>(
      options.getProperty(runtime, "channelCount").getNumber());

  size_t batchDurationSeconds = static_cast<size_t>(
      options.getProperty(runtime, "batchDurationSeconds").getNumber());

  Format format =
      static_cast<Format>(options.getProperty(runtime, "format").getNumber());

  auto presetOptions =
      options.getProperty(runtime, "preset").getObject(runtime);

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

} // namespace audioapi
