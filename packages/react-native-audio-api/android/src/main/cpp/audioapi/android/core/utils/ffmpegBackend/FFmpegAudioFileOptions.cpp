
#include <audioapi/android/core/utils/FileUtils.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegAudioFileOptions.h>

namespace audioapi {

FFmpegAudioFileOptions::FFmpegAudioFileOptions(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t flags)
    : sampleRate_(sampleRate),
      channelCount_(channelCount),
      bitRate_(bitRate),
      flags_(flags) {}

std::string FFmpegAudioFileOptions::getFileExtension() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);

  switch (format) {
    // WAV is encoded using miniaudio, so it is not necessary here
    // but lets leave it for convenience
    // when using FFmpeg we will use M4A as default format
    case android::fileutils::FileFormat::WAV:
      return "wav";
    case android::fileutils::FileFormat::CAF:
      return "caf";
    case android::fileutils::FileFormat::M4A:
      return "m4a";
    case android::fileutils::FileFormat::FLAC:
      return "flac";
    default:
      return "m4a";
  }
}

AVCodecID FFmpegAudioFileOptions::getPCMCodecID() const {
  android::fileutils::BitDepth bitDepth =
      android::fileutils::bitDepthFromFlags(flags_);

  switch (bitDepth) {
    case android::fileutils::BitDepth::BIT_16:
      return AV_CODEC_ID_PCM_S16LE;
    case android::fileutils::BitDepth::BIT_24:
      return AV_CODEC_ID_PCM_S24LE;
    case android::fileutils::BitDepth::BIT_32:
      return AV_CODEC_ID_PCM_F32LE;
    default:
      return AV_CODEC_ID_PCM_F32LE;
  }
}

AVCodecID FFmpegAudioFileOptions::getCodecID() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);

  switch (format) {
    case android::fileutils::FileFormat::WAV:
    case android::fileutils::FileFormat::CAF:
      return getPCMCodecID();
    case android::fileutils::FileFormat::M4A:
      return AV_CODEC_ID_AAC;
    case android::fileutils::FileFormat::FLAC:
      return AV_CODEC_ID_FLAC;
    default:
      return AV_CODEC_ID_AAC;
  }
}

AVSampleFormat FFmpegAudioFileOptions::getSampleFormat() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);
  android::fileutils::BitDepth bitDepth =
      android::fileutils::bitDepthFromFlags(flags_);

  if (format == android::fileutils::FileFormat::M4A) {
    return AV_SAMPLE_FMT_FLTP;
  }

  switch (bitDepth) {
    case android::fileutils::BitDepth::BIT_16:
      return AV_SAMPLE_FMT_S16;
    case android::fileutils::BitDepth::BIT_24:
      return AV_SAMPLE_FMT_S32;
    case android::fileutils::BitDepth::BIT_32:
      return AV_SAMPLE_FMT_FLT;
    default:
      return AV_SAMPLE_FMT_FLT;
  }
}

const AVCodec *FFmpegAudioFileOptions::getCodec() {
  return avcodec_find_encoder(getCodecID());
}

std::string FFmpegAudioFileOptions::getFilePath(
    const std::string &baseFileName) const {
  std::string extension = getFileExtension();

  return android::fileutils::getFilePath(
      android::fileutils::directoryFromFlags(flags_), baseFileName, extension);
}

float FFmpegAudioFileOptions::getSampleRate() const {
  return sampleRate_;
}

size_t FFmpegAudioFileOptions::getChannelCount() const {
  return channelCount_;
}

size_t FFmpegAudioFileOptions::getBitRate() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);

  if (format != android::fileutils::FileFormat::M4A) {
    return 0; // bit rate is not used for PCM formats
  } else

    return bitRate_;
}

int FFmpegAudioFileOptions::getFlacCompressionLevel() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);

  if (format != android::fileutils::FileFormat::FLAC) {
    return -1; // compression level is not used for non-FLAC formats
  }

  // TODO: extract compression level from flags
  return 5; // default compression level
}

std::string FFmpegAudioFileOptions::getMuxerName() const {
  android::fileutils::FileFormat format =
      android::fileutils::formatFromFlags(flags_);

  switch (format) {
    case android::fileutils::FileFormat::WAV:
      return "wav";
    case android::fileutils::FileFormat::CAF:
      return "caf";
    case android::fileutils::FileFormat::M4A:
      return "mp4";
    case android::fileutils::FileFormat::FLAC:
      return "flac";
    default:
      return "mp4";
  }
}

} // namespace audioapi
