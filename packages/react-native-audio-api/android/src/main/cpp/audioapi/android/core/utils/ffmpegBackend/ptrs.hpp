extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}

#include <audioapi/android/core/utils/ffmpegBackend/utils.h>

namespace audioapi::android::ffmpeg {

template<> inline void AvDtor<AVCodecContext>::operator()(AVCodecContext* ctx) const {
  if (ctx) {
    avcodec_free_context(&ctx);
  }
}

template<> inline void AvDtor<AVFormatContext>::operator()(AVFormatContext* ctx) const {
  if (ctx) {
    avformat_free_context(ctx);
  }
}

template<> inline void AvDtor<AVFrame>::operator()(AVFrame* frame) const {
  if (frame) {
    av_frame_free(&frame);
  }
}

template<> inline void AvDtor<AVPacket>::operator()(AVPacket* packet) const {
  if (packet) {
    av_packet_free(&packet);
  }
}

template<> inline void AvDtor<SwrContext>::operator()(SwrContext* ctx) const {
  if (ctx) {
    swr_free(&ctx);
  }
}

template<> inline void AvDtor<AVAudioFifo>::operator()(AVAudioFifo* fifo) const {
  if (fifo) {
    av_audio_fifo_free(fifo);
  }
}

} // namespace audioapi::android::ffmpeg
