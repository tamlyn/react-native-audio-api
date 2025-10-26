#pragma once

#include <string>

namespace audioapi::android::fileutils {

enum class FileDirectory {
  FILES_DIR,
  CACHE_DIR,
};

enum class FileFormat {
  WAV,
  CAF,
  M4A,
  FLAC,
};

enum class BitDepth {
  BIT_16,
  BIT_24,
  BIT_32,
};

bool createDirectoryIfNotExists(const std::string &directoryPath);
std::string getTimestampString();
std::string getISODateString();
FileFormat formatFromFlags(size_t flags);
FileDirectory directoryFromFlags(size_t flags);
BitDepth bitDepthFromFlags(size_t flags);
std::string getDirectory(FileDirectory dir);
std::string getFilePath(const FileDirectory &directory, const std::string &baseFileName, const std::string &extension);

} // namespace audioapi::android::fileutils
