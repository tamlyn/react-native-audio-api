#include <android/log.h>
#include <cmath>

extern "C" {
#include <libavformat/avio.h>
}

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegAudioFileOptions.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileWriter.h>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

namespace audioapi {

FFmpegAudioFileWriter::FFmpegAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags)
    : AndroidFileWriterBackend(
          sampleRate,
          channelCount,
          bitRate,
          androidFlags) {
  fileOptions_ = std::make_shared<FFmpegAudioFileOptions>(
      sampleRate, channelCount, bitRate, androidFlags);
}

FFmpegAudioFileWriter::~FFmpegAudioFileWriter() {
  isFileOpen_.store(false);
  fileOptions_.reset();
}

std::string FFmpegAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount,
    int32_t streamMaxBufferSize) {
  filePath_ = fileOptions_->getFilePath("audio");

  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  streamMaxBufferSize_ = streamMaxBufferSize;
  framesWritten_.store(0);
  nextPts_ = 0;

  const AVCodec *codec = fileOptions_->getCodec();
  AVFormatContext *rawFormatCtx = nullptr;

  if (avformat_alloc_output_context2(
          &rawFormatCtx,
          nullptr,
          fileOptions_->getMuxerName().c_str(),
          filePath_.c_str()) < 0 ||
      !rawFormatCtx) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to allocate FFmpeg format context for file: %s",
        filePath_.c_str());
    return "";
  }

  formatCtx_ = AVFormatContextPtr(rawFormatCtx);
  stream_ = avformat_new_stream(formatCtx_.get(), codec);

  encoderCtx_ = AVCodecContextPtr(avcodec_alloc_context3(codec));

  if (!encoderCtx_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to allocate FFmpeg codec context for file");
    return "";
  }

  AVDictionary *codecOptions = nullptr;
  size_t bitRate = fileOptions_->getBitRate();
  int flacCompressionLevel = fileOptions_->getFlacCompressionLevel();
  av_channel_layout_default(
      &encoderCtx_->ch_layout, fileOptions_->getChannelCount());

  encoderCtx_->sample_rate = fileOptions_->getSampleRate();
  encoderCtx_->sample_fmt = fileOptions_->getSampleFormat();

  if (bitRate > 0) {
    encoderCtx_->bit_rate = bitRate;
  }

  if (flacCompressionLevel >= 0) {
    av_dict_set_int(
        &codecOptions, "compression_level", flacCompressionLevel, 0);
  }

  if (avcodec_open2(encoderCtx_.get(), codec, &codecOptions) < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to open FFmpeg codec for file");
    av_dict_free(&codecOptions);
    return "";
  }

  av_dict_free(&codecOptions);

  if (avcodec_parameters_from_context(stream_->codecpar, encoderCtx_.get()) <
      0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to copy codec parameters to stream for file");
    return "";
  }

  if (!(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
    if (avio_open(&formatCtx_->pb, filePath_.c_str(), AVIO_FLAG_WRITE) < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to open output file: %s",
          filePath_.c_str());
      return "";
    }
  }

  stream_->time_base =
      AVRational{1, static_cast<int>(encoderCtx_->sample_rate)};

  if (avformat_write_header(formatCtx_.get(), nullptr) < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to write header to file: %s",
        filePath_.c_str());
    return "";
  }

  frame_ = AVFramePtr(av_frame_alloc());
  packet_ = AVPacketPtr(av_packet_alloc());

  resampleCtx_ = SwrContextPtr(swr_alloc());

  AVChannelLayout inChannelLayout;
  av_channel_layout_default(&inChannelLayout, streamChannelCount_);

  av_opt_set_chlayout(resampleCtx_.get(), "in_chlayout", &inChannelLayout, 0);

  av_opt_set_chlayout(
      resampleCtx_.get(), "out_chlayout", &encoderCtx_->ch_layout, 0);

  av_opt_set_int(resampleCtx_.get(), "in_sample_rate", streamSampleRate, 0);

  av_opt_set_int(
      resampleCtx_.get(), "out_sample_rate", encoderCtx_->sample_rate, 0);

  av_opt_set_sample_fmt(
      resampleCtx_.get(), "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

  av_opt_set_sample_fmt(
      resampleCtx_.get(), "out_sample_fmt", encoderCtx_->sample_fmt, 0);

  if (swr_init(resampleCtx_.get()) < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to initialize resampler for file: %s",
        filePath_.c_str());
    return "";
  }

  int contextFrameRatio = 2;

  if (encoderCtx_->frame_size > 0) {
    contextFrameRatio = static_cast<int>(std::ceil(
        static_cast<double>(streamMaxBufferSize_) /
        static_cast<double>(encoderCtx_->frame_size)));
  }

  int fifoSize = std::max(
      encoderCtx_->frame_size > 0
          ? encoderCtx_->frame_size * contextFrameRatio * 2
          : streamMaxBufferSize_ * 2,
      4096);

  audioFifo_ = AVAudioFifoPtr(av_audio_fifo_alloc(
      encoderCtx_->sample_fmt, encoderCtx_->ch_layout.nb_channels, fifoSize));

  __android_log_print(
      ANDROID_LOG_INFO,
      "FFmpegFileWriter",
      "Using audio FIFO size of %d frames",
      fifoSize);

  isFileOpen_.store(true);

  return filePath_;
}

std::tuple<double, double> FFmpegAudioFileWriter::closeFile() {
  if (!isFileOpen()) {
    return {0.0, 0.0};
  }

  isFileOpen_.store(false);

  const int frameSize =
      encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size : 512;

  while (av_audio_fifo_size(audioFifo_.get()) > 0) {
    const int chunkSize =
        std::min(av_audio_fifo_size(audioFifo_.get()), frameSize);

    frame_->nb_samples = chunkSize;
    av_channel_layout_copy(&frame_->ch_layout, &encoderCtx_->ch_layout);
    frame_->format = encoderCtx_->sample_fmt;
    frame_->sample_rate = encoderCtx_->sample_rate;

    if (av_frame_get_buffer(frame_.get(), 0) < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to allocate audio frame buffer during flushing");
      break;
    }

    int fifoReadFrameCount =
        av_audio_fifo_read(audioFifo_.get(), (void **)frame_->data, chunkSize);

    if (fifoReadFrameCount != chunkSize) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to read audio samples from FIFO during flushing");
      break;
    }

    frame_->pts = nextPts_;
    nextPts_ += chunkSize;

    if (avcodec_send_frame(encoderCtx_.get(), frame_.get()) < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to send audio frame to encoder during flushing");
      av_frame_unref(frame_.get());
      break;
    }

    av_frame_unref(frame_.get());
  }

  avcodec_send_frame(encoderCtx_.get(), nullptr);

  while (avcodec_receive_packet(encoderCtx_.get(), packet_.get()) == 0) {
    av_packet_rescale_ts(
        packet_.get(),
        AVRational{1, encoderCtx_->sample_rate},
        stream_->time_base);
    packet_->stream_index = stream_->index;
    av_interleaved_write_frame(formatCtx_.get(), packet_.get());
    av_packet_unref(packet_.get());
  }

  if (av_write_trailer(formatCtx_.get()) < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to write trailer to file: %s",
        filePath_.c_str());
  }

  double fileSizeInMB = avio_size(formatCtx_->pb) / BYTES_TO_MB;
  double durationInSeconds = getCurrentDuration();

  if (formatCtx_ && formatCtx_->pb) {
    avio_closep(&formatCtx_->pb);
  }

  resampleCtx_.reset();
  frame_.reset();
  packet_.reset();
  encoderCtx_.reset();
  formatCtx_.reset();
  audioFifo_.reset();

  filePath_ = "";
  return {fileSizeInMB, durationInSeconds};
}

bool FFmpegAudioFileWriter::writeAudioData(void *data, int numFrames) {
  __android_log_print(
      ANDROID_LOG_DEBUG,
      "FFmpegFileWriter",
      "Writing %d frames to file",
      numFrames);

  if (!isFileOpen()) {
    return false;
  }

  int outputLength = av_rescale_rnd(
      numFrames, encoderCtx_->sample_rate, streamSampleRate_, AV_ROUND_UP);

  frame_->nb_samples = outputLength;
  av_channel_layout_copy(&frame_->ch_layout, &encoderCtx_->ch_layout);
  frame_->format = encoderCtx_->sample_fmt;
  frame_->sample_rate = encoderCtx_->sample_rate;

  if (av_frame_get_buffer(frame_.get(), 0) < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to allocate audio frame buffer, outputLength: %d, format: %d, sample_rate: %d, channels: %d",
        outputLength,
        encoderCtx_->sample_fmt,
        encoderCtx_->sample_rate,
        encoderCtx_->ch_layout.nb_channels);

    return false;
  }

  const uint8_t *inputData[1] = {reinterpret_cast<const uint8_t *>(data)};

  int convertedSamples = swr_convert(
      resampleCtx_.get(), frame_->data, outputLength, inputData, numFrames);

  if (convertedSamples < 0) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to convert audio samples for file: %s",
        filePath_.c_str());
    av_frame_unref(frame_.get());
    return false;
  }

  int fifoWrittenFrameCount = av_audio_fifo_write(
      audioFifo_.get(), (void **)frame_->data, convertedSamples);

  if (fifoWrittenFrameCount < convertedSamples) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "FFmpegFileWriter",
        "Failed to write audio samples to FIFO");
    av_frame_unref(frame_.get());
    return false;
  }

  av_frame_unref(frame_.get());
  const int frameSize =
      encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size : 512;

  while (av_audio_fifo_size(audioFifo_.get()) >= frameSize) {
    frame_->nb_samples = frameSize;
    av_channel_layout_copy(&frame_->ch_layout, &encoderCtx_->ch_layout);
    frame_->format = encoderCtx_->sample_fmt;
    frame_->sample_rate = encoderCtx_->sample_rate;

    if (av_frame_get_buffer(frame_.get(), 0) < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to allocate audio frame buffer");
      return false;
    }

    int fifoReadFrameCount =
        av_audio_fifo_read(audioFifo_.get(), (void **)frame_->data, frameSize);

    if (fifoReadFrameCount != frameSize) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to read audio samples from FIFO");
      return false;
    }

    frame_->pts = nextPts_;
    nextPts_ += frameSize;

    if (avcodec_send_frame(encoderCtx_.get(), frame_.get()) < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to send audio frame to encoder");
      av_frame_unref(frame_.get());
      return false;
    }

    while (avcodec_receive_packet(encoderCtx_.get(), packet_.get()) == 0) {
      av_packet_rescale_ts(
          packet_.get(),
          AVRational{1, encoderCtx_->sample_rate},
          stream_->time_base);

      packet_->stream_index = stream_->index;

      if (av_interleaved_write_frame(formatCtx_.get(), packet_.get()) < 0) {
        __android_log_print(
            ANDROID_LOG_ERROR,
            "FFmpegFileWriter",
            "Failed to write audio packet to file");

        av_packet_unref(packet_.get());
        av_frame_unref(frame_.get());
        return false;
      }

      av_packet_unref(packet_.get());
    }

    av_frame_unref(frame_.get());
  }

  framesWritten_.fetch_add(numFrames);
  return true;
}

bool FFmpegAudioFileWriter::initializeConverterIfNeeded() {
  return false;
}

bool FFmpegAudioFileWriter::initializeEncoder() {
  return false;
}

bool FFmpegAudioFileWriter::isFileOpen() {
  return isFileOpen_.load();
}

bool FFmpegAudioFileWriter::isConverterRequired() {
  return isConverterRequired_.load();
}

} // namespace audioapi
