#pragma once

#include <string>
#include <memory>
#include <audioapi/utils/ReturnStatus.hpp>

namespace audioapi {

class AudioFileProperties;

namespace android::fileoptions {

ReturnStatus<void> createDirectoryIfNotExists(const std::string &directoryPath);
std::string getTimestampString();

std::string getDirectory(const std::shared_ptr<AudioFileProperties> &properties);
std::string getFileExtension(const std::shared_ptr<AudioFileProperties> &properties);
ReturnStatus<std::string> getFilePath(const std::shared_ptr<AudioFileProperties> &properties);

} // namespace android::fileoptions

} // namespace audioapi
