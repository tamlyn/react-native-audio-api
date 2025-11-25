#include <android/log.h>
#include <cmath>

extern "C" {
#include <libavformat/avio.h>
}

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/FileOptions.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileUtils.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileWriter.h>
#include <audioapi/utils/AudioFileProperties.hpp>
#include <audioapi/utils/ReturnStatus.hpp>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

namespace audioapi {

inline std::string getFFmpegErrorString(int errorCode) {
  char errorBuffer[AV_ERROR_MAX_STRING_SIZE];

  if (av_strerror(errorCode, errorBuffer, sizeof(errorBuffer)) < 0) {
    return "Unknown FFmpeg error: " + std::to_string(errorCode);
    ;
  }

  return std::string(errorBuffer);
}

FFmpegAudioFileWriter::FFmpegAudioFileWriter(
    std::shared_ptr<AudioFileProperties> properties)
    : AndroidFileWriterBackend(properties) {}

FFmpegAudioFileWriter::~FFmpegAudioFileWriter() {
  isFileOpen_.store(false);
}

OpenFileStatus FFmpegAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount,
    int32_t streamMaxBufferSize) {
  filePath_ = android::fileoptions::getFilePath(properties_);

  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  streamMaxBufferSize_ = streamMaxBufferSize;
  framesWritten_.store(0);
  nextPts_ = 0;
  int result = 0;

  const AVCodec *codec = android::ffmpeg::fileutils::getCodec(properties_);
  AVFormatContext *rawFormatCtx = nullptr;

  result = avformat_alloc_output_context2(
      &rawFormatCtx,
      nullptr,
      android::ffmpeg::fileutils::getMuxerName(properties_).c_str(),
      filePath_.c_str());

  if (result < 0) {
    return OpenFileStatus::Error(
        "Failed to allocate FFmpeg format context with error: " +
        getFFmpegErrorString(result));
  }

  if (!rawFormatCtx) {
    return OpenFileStatus::Error("Failed to allocate FFmpeg format context");
  }

  formatCtx_ = AVFormatContextPtr(rawFormatCtx);
  stream_ = avformat_new_stream(formatCtx_.get(), codec);
  encoderCtx_ = AVCodecContextPtr(avcodec_alloc_context3(codec));

  if (!encoderCtx_) {
    return OpenFileStatus::Error("Failed to allocate FFmpeg codec context");
  }

  AVDictionary *codecOptions = nullptr;
  size_t bitRate = properties_->bitRate;
  int flacCompressionLevel = properties_->flacCompressionLevel;

  av_channel_layout_default(&encoderCtx_->ch_layout, properties_->channelCount);

  encoderCtx_->sample_rate = properties_->sampleRate;
  encoderCtx_->sample_fmt =
      android::ffmpeg::fileutils::getSampleFormat(properties_);

  if (bitRate > 0) {
    encoderCtx_->bit_rate = bitRate;
  }

  if (flacCompressionLevel >= 0) {
    av_dict_set_int(
        &codecOptions, "compression_level", flacCompressionLevel, 0);
  }

  result = avcodec_open2(encoderCtx_.get(), codec, &codecOptions);
  if (result < 0) {
    av_dict_free(&codecOptions);
    return OpenFileStatus::Error(
        "Failed to open FFmpeg codec with error: " +
        getFFmpegErrorString(result));
  }

  av_dict_free(&codecOptions);

  result =
      avcodec_parameters_from_context(stream_->codecpar, encoderCtx_.get());

  if (result < 0) {
    return OpenFileStatus::Error(
        "Failed to copy codec parameters to stream with error: " +
        getFFmpegErrorString(result));
  }

  if (!(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
    result = avio_open(&formatCtx_->pb, filePath_.c_str(), AVIO_FLAG_WRITE);

    if (result < 0) {
      return OpenFileStatus::Error(
          "Failed to open output file with error: " +
          getFFmpegErrorString(result));
    }
  }

  stream_->time_base =
      AVRational{1, static_cast<int>(encoderCtx_->sample_rate)};

  result = avformat_write_header(formatCtx_.get(), nullptr);

  if (result < 0) {
    return OpenFileStatus::Error(
        "Failed to write header to file: " + filePath_);
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

  result = swr_init(resampleCtx_.get());

  if (result < 0) {
    return OpenFileStatus::Error(
        "Failed to initialize resampler for file: " + filePath_);
  }

  int contextFrameRatio = 4;

  if (encoderCtx_->frame_size > 0) {
    contextFrameRatio = static_cast<int>(std::ceil(
        static_cast<double>(streamMaxBufferSize_) /
        static_cast<double>(encoderCtx_->frame_size)));
  }

  int fifoSize = std::max(
      encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size * contextFrameRatio
                                  : streamMaxBufferSize_ * contextFrameRatio,
      8192);

  audioFifo_ = AVAudioFifoPtr(av_audio_fifo_alloc(
      encoderCtx_->sample_fmt, encoderCtx_->ch_layout.nb_channels, fifoSize));

  isFileOpen_.store(true);
  return OpenFileStatus::Success(filePath_);
}

CloseFileStatus FFmpegAudioFileWriter::closeFile() {
  if (!isFileOpen()) {
    return CloseFileStatus::Error("File is not open");
  }

  int result = 0;
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

    result = av_frame_get_buffer(frame_.get(), 0);

    if (result < 0) {
      return CloseFileStatus::Error(
          "Failed to allocate audio frame buffer during flushing with error:" +
          getFFmpegErrorString(result));
    }

    int fifoReadFrameCount =
        av_audio_fifo_read(audioFifo_.get(), (void **)frame_->data, chunkSize);

    if (fifoReadFrameCount != chunkSize) {
      return CloseFileStatus::Error(
          "Failed to read audio samples from FIFO during flushing");
    }

    frame_->pts = nextPts_;
    nextPts_ += chunkSize;

    result = avcodec_send_frame(encoderCtx_.get(), frame_.get());

    if (result < 0) {
      return CloseFileStatus::Error(
          "Failed to send audio frame to encoder during flushing with error: " +
          getFFmpegErrorString(result));
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

  result = av_write_trailer(formatCtx_.get());

  if (result < 0) {
    return CloseFileStatus::Error(
        "Failed to write trailer with error: " + getFFmpegErrorString(result));
  }

  double fileSizeInMB = avio_size(formatCtx_->pb) / BYTES_TO_MB;
  double durationInSeconds = getCurrentDuration();

  if (formatCtx_ && formatCtx_->pb) {
    avio_closep(&formatCtx_->pb);
  }

  filePath_ = "";
  return CloseFileStatus::Success({fileSizeInMB, durationInSeconds});
}

bool FFmpegAudioFileWriter::writeAudioData(void *data, int numFrames) {
  if (!isFileOpen()) {
    return false;
  }

  if (flushFifoToEncoder() < 0) {
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

  if (flushFifoToEncoder() < 0) {
    return false;
  }

  framesWritten_.fetch_add(numFrames);
  return true;
}

int FFmpegAudioFileWriter::flushFifoToEncoder() {
  int frameSize = encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size : 512;
  int result = 0;

  while (av_audio_fifo_size(audioFifo_.get()) >= frameSize) {
    frame_->nb_samples = frameSize;
    av_channel_layout_copy(&frame_->ch_layout, &encoderCtx_->ch_layout);
    frame_->format = encoderCtx_->sample_fmt;
    frame_->sample_rate = encoderCtx_->sample_rate;

    result = av_frame_get_buffer(frame_.get(), 0);

    if (result < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to allocate audio frame buffer during flushing");
      return result;
    }

    int fifoReadFrameCount =
        av_audio_fifo_read(audioFifo_.get(), (void **)frame_->data, frameSize);

    if (fifoReadFrameCount != frameSize) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to read audio samples from FIFO during flushing");
      return -1;
    }

    frame_->pts = nextPts_;
    nextPts_ += frameSize;

    result = avcodec_send_frame(encoderCtx_.get(), frame_.get());

    if (result < 0) {
      __android_log_print(
          ANDROID_LOG_ERROR,
          "FFmpegFileWriter",
          "Failed to send audio frame to encoder during flushing");
      av_frame_unref(frame_.get());
      return result;
    }

    while (avcodec_receive_packet(encoderCtx_.get(), packet_.get()) == 0) {
      av_packet_rescale_ts(
          packet_.get(),
          AVRational{1, encoderCtx_->sample_rate},
          stream_->time_base);
      packet_->stream_index = stream_->index;

      result = av_interleaved_write_frame(formatCtx_.get(), packet_.get());

      if (result < 0) {
        __android_log_print(
            ANDROID_LOG_ERROR,
            "FFmpegFileWriter",
            "Failed to write audio packet to file during flushing");
        av_packet_unref(packet_.get());
        av_frame_unref(frame_.get());
        return result;
      }

      avio_flush(formatCtx_.get()->pb);
      av_packet_unref(packet_.get());
    }

    av_frame_unref(frame_.get());
  }

  return 0;
}

bool FFmpegAudioFileWriter::isFileOpen() {
  return isFileOpen_.load();
}

bool FFmpegAudioFileWriter::isConverterRequired() {
  return isConverterRequired_.load();
}

} // namespace audioapi
