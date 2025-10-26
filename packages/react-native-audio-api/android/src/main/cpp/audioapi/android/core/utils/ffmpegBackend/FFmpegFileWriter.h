#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>

#include <string>
#include <memory>

namespace audioapi {

class FFmpegAudioFileOptions;

struct AVCodecContextDTOR {
  void operator()(AVCodecContext* ctx) const {
    if (ctx) {
      avcodec_free_context(&ctx);
    }
  }
};

struct AVFormatContextDTOR {
  void operator()(AVFormatContext* ctx) const {
    if (ctx) {
      avformat_free_context(ctx);
    }
  }
};

struct AVFrameDTOR {
  void operator()(AVFrame* frame) const {
    if (frame) {
      av_frame_free(&frame);
    }
  }
};

struct AVPacketDTOR {
  void operator()(AVPacket* packet) const {
    if (packet) {
      av_packet_free(&packet);
    }
  }
};

struct SwrContextDTOR {
  void operator()(SwrContext* ctx) const {
    if (ctx) {
      swr_free(&ctx);
    }
  }
};

struct AVAudioFifoDTOR {
  void operator()(AVAudioFifo* fifo) const {
    if (fifo) {
      av_audio_fifo_free(fifo);
    }
  }
};

using AVCodecContextPtr = std::unique_ptr<AVCodecContext, AVCodecContextDTOR>;
using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDTOR>;
using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDTOR>;
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDTOR>;
using SwrContextPtr = std::unique_ptr<SwrContext, SwrContextDTOR>;
using AVAudioFifoPtr = std::unique_ptr<AVAudioFifo, AVAudioFifoDTOR>;

class FFmpegAudioFileWriter : public AndroidFileWriterBackend {
 public:
  FFmpegAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags);
  ~FFmpegAudioFileWriter() override;

  void openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) override;
  std::string closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  std::shared_ptr<FFmpegAudioFileOptions> fileOptions_;

  std::atomic<bool> isFileOpen_{false};
  std::atomic<bool> isConverterRequired_{false};

  AVCodecContextPtr encoderCtx_{nullptr};
  AVFormatContextPtr formatCtx_{nullptr};
  SwrContextPtr resampleCtx_{nullptr};
  AVPacketPtr packet_{nullptr};
  AVFramePtr frame_{nullptr};
  AVStream* stream_{nullptr};
  AVAudioFifoPtr audioFifo_{nullptr};
  int64_t nextPts_;

  bool initializeConverterIfNeeded();
  bool initializeEncoder();

  bool isFileOpen();
  bool isConverterRequired();
};

} // namespace audioapi
