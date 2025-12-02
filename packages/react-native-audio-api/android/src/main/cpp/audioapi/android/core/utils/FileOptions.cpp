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

bool createDirectoryIfNotExists(const std::string &directoryPath) {
  std::error_code ec;

  if (!std::filesystem::exists(directoryPath, ec)) {
    bool created = std::filesystem::create_directories(directoryPath, ec);

    if (ec) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FileOptions",
          "Error creating directory at path: %s, error: %s",
          directoryPath.c_str(),
          ec.message().c_str());

      return false;
    }

    return created;
  }

  if (ec) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FileOptions",
        "Error checking existence of directory at path: %s, error: %s",
        directoryPath.c_str(),
        ec.message().c_str());

    return false;
  }

  return true;
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

std::string getFilePath(const std::shared_ptr<AudioFileProperties> &properties) {
  std::string directory = getDirectory(properties);
  std::string subDirectory = std::format("{}/{}", directory, properties->subDirectory);
  std::string fileTimestamp = getTimestampString();
  std::string extension = getFileExtension(properties);

  if (!createDirectoryIfNotExists(subDirectory)) {
    return "";
  }

  return std::format(
      "{}/{}_{}.{}", subDirectory, properties->fileNamePrefix, fileTimestamp, extension);
}

} // namespace audioapi::android::fileoptions
