#pragma once

#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/ffmpegBackend/utils.h>
#include <string>
#include <memory>
#include <tuple>
#include <chrono>
#include <audioapi/utils/ReturnStatus.hpp>

struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct AVAudioFifo;
struct SwrContext;
struct AVStream;

namespace audioapi {

class AudioFileProperties;

namespace android::ffmpeg {

class FFmpegAudioFileWriter : public AndroidFileWriterBackend {
 public:
  explicit FFmpegAudioFileWriter(std::shared_ptr<AudioFileProperties> properties);
  ~FFmpegAudioFileWriter() override;

  OpenFileStatus openFile(int32_t streamSampleRate, int32_t streamChannelCount, int32_t streamMaxBufferSize) override;
  CloseFileStatus closeFile() override;

  bool writeAudioData(void *data, int numFrames) override;

 private:
  std::atomic<bool> isFileOpen_{false};
  std::atomic<bool> isConverterRequired_{false};

  av_unique_ptr<AVCodecContext> encoderCtx_{nullptr};
  av_unique_ptr<AVFormatContext> formatCtx_{nullptr};
  av_unique_ptr<SwrContext> resampleCtx_{nullptr};
  av_unique_ptr<AVPacket> packet_{nullptr};
  av_unique_ptr<AVFrame> frame_{nullptr};
  AVStream* stream_{nullptr};
  av_unique_ptr<AVAudioFifo> audioFifo_{nullptr};
  unsigned int nextPts_;

  std::chrono::steady_clock::time_point lastFlushTime_ = std::chrono::steady_clock::now();
  uint32_t flushIntervalMs_ = 500;

  bool isFileOpen();
  bool isConverterRequired();

  // Initialization helper methods
  ReturnStatus<void> initializeFormatContext(const AVCodec* codec);
  ReturnStatus<void> configureAndOpenCodec(const AVCodec* codec);
  ReturnStatus<void> initializeStream();
  ReturnStatus<void> openIOAndWriteHeader();
  ReturnStatus<void> initializeResampler(int32_t inputRate, int32_t inputChannels);
  void initializeBuffers(int32_t maxBufferSize);

  // Processing helper methods
  bool resampleAndPushToFifo(void *data, int numFrames);
  int processFifo(bool flush);
  int writeEncodedPackets();
  int prepareFrameForEncoding(int samplesToRead);

  // Finalization helper methods
  CloseFileStatus finalizeOutput();
};

} // namespace android::ffmpeg

} // namespace audioapi
