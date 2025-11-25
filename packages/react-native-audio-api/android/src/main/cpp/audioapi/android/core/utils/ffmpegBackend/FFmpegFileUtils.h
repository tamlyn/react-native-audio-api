#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace audioapi {

class AudioFileProperties;

namespace android::ffmpeg::fileutils {

AVCodecID getPCMCodecID(const std::shared_ptr<AudioFileProperties> &properties);
AVCodecID getCodecID(const std::shared_ptr<AudioFileProperties> &properties);
AVSampleFormat getSampleFormat(const std::shared_ptr<AudioFileProperties> &properties);
const AVCodec* getCodec(const std::shared_ptr<AudioFileProperties> &properties);
std::string getMuxerName(const std::shared_ptr<AudioFileProperties> &properties);

} // namespace android::ffmpeg::fileutils

} // namespace audioapi
