#include <android/log.h>
#include <audioapi/android/core/AndroidAudioRecorder.h>
#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/AndroidRecorderCallback.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileWriter.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/core/sources/RecorderAdapterNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

AndroidAudioRecorder::AndroidAudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry) {
  AudioStreamBuilder builder;

  builder.setSharingMode(SharingMode::Exclusive)
      ->setDirection(Direction::Input)
      ->setFormat(AudioFormat::Float)
      ->setFormatConversionAllowed(true)
      ->setPerformanceMode(PerformanceMode::None)
      ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
      ->setDataCallback(this)
      ->openStream(mStream_);

  streamSampleRate_ = mStream_->getSampleRate();
  streamChannelCount_ = mStream_->getChannelCount();
  streamMaxBufferSizeInFrames_ = mStream_->getBufferSizeInFrames();
  nativeAudioRecorder_ = jni::make_global(NativeAudioRecorder::create());
}

AndroidAudioRecorder::~AndroidAudioRecorder() {
  nativeAudioRecorder_.release();

  if (mStream_) {
    mStream_->requestStop();
    mStream_->close();
    mStream_.reset();
  }
}

// jni::ThreadScope::WithClassLoader(
//     [this]() { nativeAudioRecorder_->start(); });
// mStream_->requestStart();

std::string AndroidAudioRecorder::start() {
  if (isRecording()) {
    return filePath_;
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return filePath_;
  }

  if (usesFileOutput()) {
    filePath_ = fileWriter_->openFile(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (usesCallback()) {
    // TODO: create circular buffer and converter?
    callback_->prepare(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  nativeAudioRecorder_->start();

  jni::ThreadScope::WithClassLoader(
      [this]() { nativeAudioRecorder_->start(); });

  state_.store(RecorderState::Recording);

  return filePath_;
}

std::tuple<std::string, double, double> AndroidAudioRecorder::stop() {
  std::string filePath = filePath_;
  double outputFileSize = 0.0;
  double outputDuration = 0.0;

  if (!isRecording()) {
    return {filePath_, 0.0, 0.0};
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return {filePath_, 0.0, 0.0};
  }

  jni::ThreadScope::WithClassLoader(
    [this]() { nativeAudioRecorder_->stop(); });

  mStream_->requestStop();
  state_.store(RecorderState::Idle);

  if (usesFileOutput()) {
    auto [size, duration] = fileWriter_->closeFile();
    outputFileSize = size;
    outputDuration = duration;
  }

  if (usesCallback()) {
    callback_->cleanup();
  }

  filePath_ = "";
  return {filePath, outputFileSize, outputDuration};
}

void AndroidAudioRecorder::enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags) {
  uint8_t format = static_cast<uint8_t>(androidFlags & 0xF);

  if (format == 1) {
    fileWriter_ = std::make_shared<MiniAudioFileWriter>(
        sampleRate, channelCount, bitRate, androidFlags);
  } else {
    fileWriter_ = std::make_shared<FFmpegAudioFileWriter>(
        sampleRate, channelCount, bitRate, androidFlags);
  }

  fileOutputEnabled_.store(true);
}

void AndroidAudioRecorder::disableFileOutput() {
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
}

void AndroidAudioRecorder::pause() {
  if (!isRecording()) {
    return;
  }

  mStream_->pause(0);
  state_.store(RecorderState::Paused);
}

void AndroidAudioRecorder::resume() {
  if (!isPaused()) {
    return;
  }

  mStream_->start(0);
  state_.store(RecorderState::Recording);
}

void AndroidAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId) {
  callback_ = std::make_shared<AndroidRecorderCallback>(
      audioEventHandlerRegistry_,
      sampleRate,
      bufferLength,
      channelCount,
      callbackId);
  callbackOutputEnabled_.store(true);
}

void AndroidAudioRecorder::clearOnAudioReadyCallback() {
  callbackOutputEnabled_.store(false);
  callback_ = nullptr;
}

DataCallbackResult AndroidAudioRecorder::onAudioReady(
    oboe::AudioStream *oboeStream,
    void *audioData,
    int32_t numFrames) {
  if (isPaused()) {
    return DataCallbackResult::Continue;
  }

  if (usesFileOutput()) {
    fileWriter_->writeAudioData(audioData, numFrames);
  }

  if (usesCallback()) {
    callback_->receiveAudioData(audioData, numFrames);
  }

  return DataCallbackResult::Continue;
}

double AndroidAudioRecorder::getCurrentDuration() const {
  if (usesFileOutput()) {
    return fileWriter_->getCurrentDuration();
  }

  return 0.0;
}

} // namespace audioapi
