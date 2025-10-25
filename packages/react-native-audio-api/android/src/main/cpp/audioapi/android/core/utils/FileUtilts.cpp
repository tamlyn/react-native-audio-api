
#include <android/log.h>
#include <audioapi/android/core/utils/FileUtils.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>

namespace audioapi::android::fileutils {

bool createDirectoryIfNotExists(const std::string &directoryPath) {
  std::error_code ec;

  if (!std::filesystem::exists(directoryPath, ec)) {
    bool created = std::filesystem::create_directories(directoryPath, ec);

    if (ec) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FileUtils",
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
        "FileUtils",
        "Error checking existence of directory at path: %s, error: %s",
        directoryPath.c_str(),
        ec.message().c_str());

    return false;
  }

  return true;
}

std::string getTimestampString() {
  auto tNow = std::chrono::system_clock::now();
  return std::format(
      "{:%Y%m%d_%H%M%S}", std::chrono::floor<std::chrono::seconds>(tNow));
}

std::string getISODateString() {
  auto tNow = std::chrono::system_clock::now();
  return std::format(
      "{:%Y-%m-%d}", std::chrono::floor<std::chrono::days>(tNow));
}

FileDirectory directoryFromFlag(uint8_t directoryFlag) {
  switch (directoryFlag) {
    case 1:
      return FileDirectory::FILES_DIR;
    case 2:
      return FileDirectory::CACHE_DIR;
    default:
      return FileDirectory::CACHE_DIR;
  }
}

std::string getDirectory(FileDirectory dir) {
  switch (dir) {
    case FileDirectory::FILES_DIR:
      return NativeFileInfo::getFilesDir();
    case FileDirectory::CACHE_DIR:
      return NativeFileInfo::getCacheDir();
    default:
      return NativeFileInfo::getCacheDir();
  }
}

std::string getFilePath(
    const FileDirectory &directory,
    const std::string &baseFileName,
    const std::string &extension) {
  std::string basePath = getDirectory(directory);
  std::string subDirectory = getISODateString();
  std::string fileTimestamp = getTimestampString();

  std::string subDirectoryPath =
      std::format("{}/AudioAPI/{}", basePath, subDirectory);

  if (!createDirectoryIfNotExists(subDirectoryPath)) {
    return "";
  }

  return std::format(
      "{}/{}_{}.{}", subDirectoryPath, baseFileName, fileTimestamp, extension);
}

} // namespace audioapi::android::fileutils
