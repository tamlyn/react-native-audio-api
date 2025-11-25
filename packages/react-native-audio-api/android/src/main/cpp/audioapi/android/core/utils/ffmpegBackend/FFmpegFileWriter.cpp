
#include <android/log.h>
// #include <cmath>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/FileOptions.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileWriter.h>
#include <audioapi/android/core/utils/ffmpegBackend/ptrs.hpp>
#include <audioapi/android/core/utils/ffmpegBackend/utils.h>
#include <audioapi/utils/AudioFileProperties.h>

constexpr double BYTES_TO_MB = 1024.0 * 1024.0;

#define LOG_TAG "FFmpegFileWriter"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace audioapi::android::ffmpeg {

FFmpegAudioFileWriter::FFmpegAudioFileWriter(
    std::shared_ptr<AudioFileProperties> properties)
    : AndroidFileWriterBackend(properties) {}

FFmpegAudioFileWriter::~FFmpegAudioFileWriter() {}

OpenFileStatus FFmpegAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount,
    int32_t streamMaxBufferSize) {
  filePath_ = fileoptions::getFilePath(properties_);
  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  streamMaxBufferSize_ = streamMaxBufferSize;
  framesWritten_.store(0);
  nextPts_ = 0;
  ReturnStatus<void> status;

  const AVCodec *codec = getCodec(properties_);

  if (!codec) {
    return OpenFileStatus::Error("Unsupported codec for the given file format");
  }

  status = initializeFormatContext(codec);

  if (!status.isSuccess()) {
    return OpenFileStatus::Error(status.getMessage());
  }

  status = configureAndOpenCodec(codec);

  if (!status.isSuccess()) {
    return OpenFileStatus::Error(status.getMessage());
  }

  status = initializeStream();

  if (!status.isSuccess()) {
    return OpenFileStatus::Error(status.getMessage());
  }

  status = openIOAndWriteHeader();

  if (!status.isSuccess()) {
    return OpenFileStatus::Error(status.getMessage());
  }

  status = initializeResampler(streamSampleRate, streamChannelCount);

  if (!status.isSuccess()) {
    return OpenFileStatus::Error(status.getMessage());
  }

  initializeBuffers(streamMaxBufferSize);

  isFileOpen_.store(true);
  return OpenFileStatus::Success(filePath_);
}

CloseFileStatus FFmpegAudioFileWriter::closeFile() {
  if (!isFileOpen()) {
    return CloseFileStatus::Error("File is not open");
  }

  if (processFifo(true) < 0) {
    return CloseFileStatus::Error("Failed to flush FIFO to encoder");
  }

  int result = avcodec_send_frame(encoderCtx_.get(), nullptr);
  if (result < 0) {
    return CloseFileStatus::Error("Failed to send EOF to encoder");
  }

  if (writeEncodedPackets() < 0) {
    return CloseFileStatus::Error("Failed to drain encoder packets");
  }

  return finalizeOutput();
}

bool FFmpegAudioFileWriter::writeAudioData(void *data, int numFrames) {
  if (!isFileOpen()) {
    return false;
  }

  if (!resampleAndPushToFifo(data, numFrames)) {
    return false;
  }

  framesWritten_.fetch_add(numFrames);

  if (processFifo(false) < 0) {
    return false;
  }

  return true;
}

bool FFmpegAudioFileWriter::isFileOpen() {
  return isFileOpen_.load();
}

bool FFmpegAudioFileWriter::isConverterRequired() {
  return isConverterRequired_.load();
}

ReturnStatus<void> FFmpegAudioFileWriter::initializeFormatContext(
    const AVCodec *codec) {
  AVFormatContext *rawFormatCtx = nullptr;

  int result = avformat_alloc_output_context2(
      &rawFormatCtx,
      nullptr,
      getMuxerName(properties_).c_str(),
      filePath_.c_str());

  if (result < 0 || !rawFormatCtx) {
    return ReturnStatus<void>::Error(
        "Failed to allocate FFmpeg format context with error: " +
        parseErrorCode(result));
  }

  formatCtx_ = av_unique_ptr<AVFormatContext>(rawFormatCtx);
  return ReturnStatus<void>::Success();
}

ReturnStatus<void> FFmpegAudioFileWriter::configureAndOpenCodec(
    const AVCodec *codec) {
  encoderCtx_ = av_unique_ptr<AVCodecContext>(avcodec_alloc_context3(codec));

  if (!encoderCtx_) {
    return ReturnStatus<void>::Error("Failed to allocate FFmpeg codec context");
  }

  av_channel_layout_default(&encoderCtx_->ch_layout, properties_->channelCount);
  encoderCtx_->sample_rate = properties_->sampleRate;
  encoderCtx_->sample_fmt = getSampleFormat(properties_);

  if (properties_->bitRate > 0) {
    encoderCtx_->bit_rate = properties_->bitRate;
  }

  AVDictionary *codecOptions = nullptr;

  if (properties_->flacCompressionLevel >= 0) {
    av_dict_set_int(
        &codecOptions,
        "compression_level",
        properties_->flacCompressionLevel,
        0);
  }

  int result = avcodec_open2(encoderCtx_.get(), codec, &codecOptions);
  av_dict_free(&codecOptions);

  if (result < 0) {
    return ReturnStatus<void>::Error(
        "Failed to open FFmpeg codec with error: " + parseErrorCode(result));
  }

  return ReturnStatus<void>::Success();
}

ReturnStatus<void> FFmpegAudioFileWriter::initializeStream() {
  stream_ = avformat_new_stream(formatCtx_.get(), nullptr);

  if (!stream_) {
    return ReturnStatus<void>::Error(
        "Failed to create new stream in format context");
  }

  int result =
      avcodec_parameters_from_context(stream_->codecpar, encoderCtx_.get());

  if (result < 0) {
    return ReturnStatus<void>::Error(
        "Failed to copy codec parameters to stream with error: " +
        parseErrorCode(result));
  }

  stream_->time_base =
      AVRational{1, static_cast<int>(encoderCtx_->sample_rate)};
  return ReturnStatus<void>::Success();
}

ReturnStatus<void> FFmpegAudioFileWriter::openIOAndWriteHeader() {
  int result = 0;

  if (!(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
    result = avio_open(&formatCtx_->pb, filePath_.c_str(), AVIO_FLAG_WRITE);

    if (result < 0) {
      return ReturnStatus<void>::Error(
          "Failed to open output file with error: " + parseErrorCode(result));
    }
  }

  result = avformat_write_header(formatCtx_.get(), nullptr);

  if (result < 0) {
    return ReturnStatus<void>::Error(
        "Failed to write header to file: " + filePath_);
  }

  return ReturnStatus<void>::Success();
}

ReturnStatus<void> FFmpegAudioFileWriter::initializeResampler(
    int32_t inputRate,
    int32_t inputChannels) {
  resampleCtx_ = av_unique_ptr<SwrContext>(swr_alloc());

  if (!resampleCtx_) {
    return ReturnStatus<void>::Error("Failed to allocate resampler context");
  }

  AVChannelLayout inChannelLayout;
  av_channel_layout_default(&inChannelLayout, inputChannels);

  av_opt_set_chlayout(resampleCtx_.get(), "in_chlayout", &inChannelLayout, 0);
  av_opt_set_chlayout(
      resampleCtx_.get(), "out_chlayout", &encoderCtx_->ch_layout, 0);

  av_opt_set_int(resampleCtx_.get(), "in_sample_rate", inputRate, 0);
  av_opt_set_int(
      resampleCtx_.get(), "out_sample_rate", encoderCtx_->sample_rate, 0);

  av_opt_set_sample_fmt(
      resampleCtx_.get(), "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
  av_opt_set_sample_fmt(
      resampleCtx_.get(), "out_sample_fmt", encoderCtx_->sample_fmt, 0);

  int result = swr_init(resampleCtx_.get());

  if (result < 0) {
    return ReturnStatus<void>::Error(
        "Failed to initialize resampler for file: " + parseErrorCode(result));
  }

  return ReturnStatus<void>::Success();
}

void FFmpegAudioFileWriter::initializeBuffers(int32_t maxBufferSize) {
  frame_ = av_unique_ptr<AVFrame>(av_frame_alloc());
  packet_ = av_unique_ptr<AVPacket>(av_packet_alloc());

  int contextFrameRatio = 4;

  if (encoderCtx_->frame_size > 0) {
    contextFrameRatio = static_cast<int>(std::ceil(
        static_cast<double>(maxBufferSize) /
        static_cast<double>(encoderCtx_->frame_size)));
  }

  int calculatedSize =
      (encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size * contextFrameRatio
                                   : maxBufferSize * contextFrameRatio);

  int fifoSize = std::max(calculatedSize, 8192);

  audioFifo_ = av_unique_ptr<AVAudioFifo>(av_audio_fifo_alloc(
      encoderCtx_->sample_fmt, encoderCtx_->ch_layout.nb_channels, fifoSize));
}

bool FFmpegAudioFileWriter::resampleAndPushToFifo(
    void *inputData,
    int inputFrameCount) {
  int outputLength = av_rescale_rnd(
      inputFrameCount,
      encoderCtx_->sample_rate,
      streamSampleRate_,
      AV_ROUND_UP);

  if (prepareFrameForEncoding(outputLength) < 0) {
    LOGE("Failed to alloc conversion frame");
    return false;
  }

  const uint8_t *inputs[1] = {reinterpret_cast<const uint8_t *>(inputData)};

  int convertedSamples = swr_convert(
      resampleCtx_.get(), frame_->data, outputLength, inputs, inputFrameCount);

  if (convertedSamples < 0) {
    LOGE("Swr conversion failed");

    av_frame_unref(frame_.get());
    return false;
  }

  int written = av_audio_fifo_write(
      audioFifo_.get(), (void **)frame_->data, convertedSamples);

  av_frame_unref(frame_.get());

  if (written < convertedSamples) {
    LOGE("FIFO write failed");
    return false;
  }

  return true;
}

int FFmpegAudioFileWriter::processFifo(bool flush) {
  int frameSize = encoderCtx_->frame_size > 0 ? encoderCtx_->frame_size : 512;

  while (av_audio_fifo_size(audioFifo_.get()) >= (flush ? 1 : frameSize)) {
    const int chunkSize =
        std::min(av_audio_fifo_size(audioFifo_.get()), frameSize);

    if (prepareFrameForEncoding(chunkSize) < 0) {
      return -1;
    }

    if (av_audio_fifo_read(
            audioFifo_.get(), (void **)frame_->data, chunkSize) != chunkSize) {
      LOGE("FIFO read failed");
      return -1;
    }

    frame_->pts = nextPts_;
    nextPts_ += chunkSize;

    int result = avcodec_send_frame(encoderCtx_.get(), frame_.get());
    av_frame_unref(frame_.get()); // Done with this frame

    if (result < 0) {
      LOGE("Send frame failed: %s", parseErrorCode(result).c_str());
      return result;
    }

    if (writeEncodedPackets() < 0) {
      return -1;
    }
  }

  return 0;
}

int FFmpegAudioFileWriter::writeEncodedPackets() {
  int result = 0;

  while (true) {
    result = avcodec_receive_packet(encoderCtx_.get(), packet_.get());

    if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
      return 0;
    } else if (result < 0) {
      LOGE("Receive packet failed");
      return result;
    }

    av_packet_rescale_ts(
        packet_.get(),
        AVRational{1, encoderCtx_->sample_rate},
        stream_->time_base);
    packet_->stream_index = stream_->index;

    // Write to container
    result = av_interleaved_write_frame(formatCtx_.get(), packet_.get());

    // Flush IO buffer (optional, but good for real-time safety)
    if (formatCtx_->pb) {
      avio_flush(formatCtx_->pb);
    }

    av_packet_unref(packet_.get());

    if (result < 0) {
      LOGE("Write frame failed");
      return result;
    }
  }
}

int FFmpegAudioFileWriter::prepareFrameForEncoding(int samplesToRead) {
  frame_->nb_samples = samplesToRead;
  frame_->format = encoderCtx_->sample_fmt;
  frame_->sample_rate = encoderCtx_->sample_rate;

  int ret = av_channel_layout_copy(&frame_->ch_layout, &encoderCtx_->ch_layout);

  if (ret < 0) {
    return ret;
  }

  ret = av_frame_get_buffer(frame_.get(), 0);

  if (ret < 0) {
    LOGE("Frame alloc failed");
  }

  return ret;
}

CloseFileStatus FFmpegAudioFileWriter::finalizeOutput() {
  int result = av_write_trailer(formatCtx_.get());

  if (result < 0) {
    return CloseFileStatus::Error(
        "Failed to write trailer: " + parseErrorCode(result));
  }

  double fileSizeInMB = 0;

  if (formatCtx_->pb) {
    fileSizeInMB = avio_size(formatCtx_->pb) / BYTES_TO_MB;
    avio_closep(&formatCtx_->pb);
  }

  double durationInSeconds = getCurrentDuration();

  filePath_ = "";
  isFileOpen_.store(false);

  return CloseFileStatus::Success({fileSizeInMB, durationInSeconds});
}

} // namespace audioapi::android::ffmpeg
