#pragma once

#include <string>
#include <memory>

namespace audioapi {

class AudioFileProperties;

namespace android::fileoptions {

bool createDirectoryIfNotExists(const std::string &directoryPath);
std::string getTimestampString();

std::string getDirectory(const std::shared_ptr<AudioFileProperties> &properties);
std::string getFileExtension(const std::shared_ptr<AudioFileProperties> &properties);
std::string getFilePath(const std::shared_ptr<AudioFileProperties> &properties);

} // namespace android::fileoptions

} // namespace audioapi
