#pragma once


namespace audioapi::android::fileutils {

enum class FileDirectory {
  FILES_DIR,
  CACHE_DIR
}

bool createDirectoryIfNotExists(const std::string &directoryPath);
std::string getTimestampString();
std::string getISODateString();
FileDirectory directoryFromFlag(uint8_t directoryFlag);
std::string getDirectory(FileDirectory dir);
std::string getFilePath(const FileDirectory &directory, const std::string &baseFileName, const std::string &extension);

} // namespace audioapi::android::fileutils
