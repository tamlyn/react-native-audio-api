#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <cstddef>
#include <cstdint>
#include <string>

namespace audioapi {

class FFmpegAudioFileOptions {
 public:
  FFmpegAudioFileOptions(float sampleRate, size_t channelCount, size_t bitRate, size_t flags);
  ~FFmpegAudioFileOptions() = default;

  std::string getFileExtension() const;
  std::string getFilePath(const std::string &baseFileName) const;

  float getSampleRate() const;
  size_t getChannelCount() const;

  AVCodecID getPCMCodecID() const;
  AVCodecID getCodecID() const;
  AVSampleFormat getSampleFormat() const;
  const AVCodec* getCodec();
  size_t getBitRate() const;
  int getFlacCompressionLevel() const;
  std::string getMuxerName() const;

 private:
  float sampleRate_;
  size_t channelCount_;
  size_t flags_;
  size_t bitRate_;
};

} // namespace audioapi
