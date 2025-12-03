#include <android/log.h>
#include <audioapi/android/core/utils/FileOptions.h>
#include <audioapi/android/system/NativeFileInfo.hpp>
#include <audioapi/utils/AudioFileProperties.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <string>

namespace audioapi::android::fileoptions {

ReturnStatus<void> createDirectoryIfNotExists(const std::string &directoryPath) {
  std::error_code ec;

  if (std::filesystem::exists(directoryPath, ec)) {
    return ReturnStatus<void>::Success();
  }

  bool created = std::filesystem::create_directories(directoryPath, ec);

  if (!created) {
    return ReturnStatus<void>::Error("Failed to create directory: " + directoryPath);
  }

  if (ec) {
    return ReturnStatus<void>::Error(ec.message());
  }

  return ReturnStatus<void>::Success();
}

std::string getTimestampString() {
  auto tNow = std::chrono::system_clock::now();
  return std::format("{:%Y%m%d_%H%M%S}", std::chrono::floor<std::chrono::seconds>(tNow));
}

std::string getDirectory(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->directory) {
    case AudioFileProperties::FileDirectory::Document:
      return NativeFileInfo::getFilesDir();
    case AudioFileProperties::FileDirectory::Cache:
      return NativeFileInfo::getCacheDir();
    default:
      return NativeFileInfo::getCacheDir();
  }
}

std::string getFileExtension(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->format) {
    case AudioFileProperties::Format::WAV:
      return "wav";
    case AudioFileProperties::Format::CAF:
      return "caf";
    case AudioFileProperties::Format::M4A:
      return "m4a";
    case AudioFileProperties::Format::FLAC:
      return "flac";
    default:
      return "m4a";
  }
}

ReturnStatus<std::string> getFilePath(const std::shared_ptr<AudioFileProperties> &properties) {
  std::string directory = getDirectory(properties);
  std::string subDirectory = std::format("{}/{}", directory, properties->subDirectory);
  std::string fileTimestamp = getTimestampString();
  std::string extension = getFileExtension(properties);

  auto result = createDirectoryIfNotExists(directory);

  if (!result.isSuccess()) {
    return ReturnStatus<std::string>::Error(result.getMessage());
  }

  auto filePath = std::format(
      "{}/{}_{}.{}", subDirectory, properties->fileNamePrefix, fileTimestamp, extension);
  return ReturnStatus<std::string>::Success(filePath);
}

} // namespace audioapi::android::fileoptions
