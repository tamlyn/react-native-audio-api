extern "C" {
#include <libavcodec/avcodec.h>
}

#include <audioapi/android/core/utils/ffmpegBackend/utils.h>

#include <audioapi/android/core/utils/FileOptions.h>
#include <audioapi/utils/AudioFileProperties.h>

#include <memory>
#include <string>

namespace audioapi::android::ffmpeg {

AVCodecID getPCMCodecID(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->bitDepth) {
    case AudioFileProperties::BitDepth::Bit16:
      return AV_CODEC_ID_PCM_S16LE;
    case AudioFileProperties::BitDepth::Bit24:
      return AV_CODEC_ID_PCM_S24LE;
    case AudioFileProperties::BitDepth::Bit32:
      return AV_CODEC_ID_PCM_F32LE;
    default:
      return AV_CODEC_ID_PCM_F32LE;
  }
}

AVCodecID getCodecID(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->format) {
    case AudioFileProperties::Format::WAV:
    case AudioFileProperties::Format::CAF:
      return getPCMCodecID(properties);
    case AudioFileProperties::Format::M4A:
      return AV_CODEC_ID_AAC;
    case AudioFileProperties::Format::FLAC:
      return AV_CODEC_ID_FLAC;
    default:
      return AV_CODEC_ID_AAC;
  }
}

AVSampleFormat getSampleFormat(const std::shared_ptr<AudioFileProperties> &properties) {
  if (properties->format == AudioFileProperties::Format::M4A) {
    return AV_SAMPLE_FMT_FLTP;
  }

  switch (properties->bitDepth) {
    case AudioFileProperties::BitDepth::Bit16:
      return AV_SAMPLE_FMT_S16;
    case AudioFileProperties::BitDepth::Bit24:
      return AV_SAMPLE_FMT_S32;
    case AudioFileProperties::BitDepth::Bit32:
      return AV_SAMPLE_FMT_FLT;
    default:
      return AV_SAMPLE_FMT_FLT;
  }
}

const AVCodec *getCodec(const std::shared_ptr<AudioFileProperties> &properties) {
  return avcodec_find_encoder(getCodecID(properties));
}

std::string getMuxerName(const std::shared_ptr<AudioFileProperties> &properties) {
  switch (properties->format) {
    case AudioFileProperties::Format::WAV:
      return "wav";
    case AudioFileProperties::Format::CAF:
      return "caf";
    case AudioFileProperties::Format::M4A:
      return "mp4";
    case AudioFileProperties::Format::FLAC:
      return "flac";
    default:
      return "mp4";
  }
}

std::string parseErrorCode(int errorCode) {
  char errorBuffer[AV_ERROR_MAX_STRING_SIZE];

  if (av_strerror(errorCode, errorBuffer, sizeof(errorBuffer)) < 0) {
    return "Unknown FFmpeg error: " + std::to_string(errorCode);
  }

  return std::string(errorBuffer);
}

} // namespace audioapi::android::ffmpeg
