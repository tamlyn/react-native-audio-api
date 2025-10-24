
#include <android/log.h>
#include <audioapi/android/core/utils/AndroidAudioFileOptions.h>
#include <audioapi/android/system/NativeFileInfo.hpp>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>

namespace audioapi {

AndroidAudioFileOptions::AndroidAudioFileOptions(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t flags) {
  sampleRate_ = sampleRate;
  channelCount_ = channelCount;
  bitRate_ = bitRate;

  format_ = static_cast<uint8_t>(flags & 0xF);
  directory_ = static_cast<uint8_t>((flags >> 4) & 0xF);
  bitDepth_ = static_cast<uint8_t>((flags >> 8) & 0xF);
}

AudioFormat AndroidAudioFileOptions::getFormat() const {
  return static_cast<AudioFormat>(format_);
}

std::string AndroidAudioFileOptions::getFileExtension() const {
  switch (getFormat()) {
    case AudioFormat::WAV:
      return "wav";
    case AudioFormat::CAF:
      return "caf";
    case AudioFormat::M4A:
      return "m4a";
    case AudioFormat::FLAC:
      return "flac";
    default:
      return "wav";
  }
}

uint32_t AndroidAudioFileOptions::getBitDepth() const {
  return static_cast<uint32_t>(bitDepth_);
}

std::string AndroidAudioFileOptions::getDirectory() const {
  switch (directory_) {
    case 1:
      return NativeFileInfo::getFilesDir();
    case 2:
      return NativeFileInfo::getCacheDir();
    default:
      return NativeFileInfo::getCacheDir();
  }
}

float AndroidAudioFileOptions::getSampleRate() const {
  return sampleRate_;
}

size_t AndroidAudioFileOptions::getChannelCount() const {
  return channelCount_;
}

size_t AndroidAudioFileOptions::getBitRate() const {
  return bitRate_;
}

std::string AndroidAudioFileOptions::getFilePath(
    const std::string &baseFileName) const {
  std::string directory = getDirectory();
  std::string extension = getFileExtension();
  std::string directoryDate = getISODateStringForDirectory();
  std::string timestamp = getTimestampForFileName();

  std::string directoryPath =
      std::format("{}/AudioAPI/{}", directory, directoryDate);

  if (!ensureDirectoryExists(directoryPath)) {
    return "";
  }

  return std::format(
      "{}/{}_{}.{}", directoryPath, baseFileName, timestamp, extension);
}

std::string AndroidAudioFileOptions::getISODateStringForDirectory() const {
  auto tNow = std::chrono::system_clock::now();
  return std::format("{:%Y-%m-%d}", floor<std::chrono::days>(tNow));
}

std::string AndroidAudioFileOptions::getTimestampForFileName() const {
  auto tNow = std::chrono::system_clock::now();
  return std::format("{:%Y%m%d_%H%M%S}", floor<std::chrono::seconds>(tNow));
}

bool AndroidAudioFileOptions::ensureDirectoryExists(std::string &path) const {
  std::error_code ec;

  if (!std::filesystem::exists(path, ec)) {
    return std::filesystem::create_directories(path, ec);
  }

  return true;
}

} // namespace audioapi
