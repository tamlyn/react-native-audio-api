#include <android/log.h>
#include <audioapi/android/core/AndroidAudioRecorder.h>
#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/AndroidRecorderCallback.h>
#include <audioapi/android/core/utils/ffmpegBackend/FFmpegFileWriter.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/core/sources/RecorderAdapterNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/core/utils/Locker.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

AndroidAudioRecorder::AndroidAudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry) {
  nativeAudioRecorder_ = jni::make_global(NativeAudioRecorder::create());

  streamSampleRate_ = 0;
  streamChannelCount_ = 0;
  streamMaxBufferSizeInFrames_ = 0;
}

AndroidAudioRecorder::~AndroidAudioRecorder() {
  nativeAudioRecorder_.release();

  if (mStream_) {
    mStream_->requestStop();
    mStream_->close();
    mStream_.reset();
  }
}

bool AndroidAudioRecorder::openAudioStream() {
  if (mStream_) {
    return true;
  }

  AudioStreamBuilder builder;
  builder.setSharingMode(SharingMode::Exclusive)
      ->setDirection(Direction::Input)
      ->setFormat(AudioFormat::Float)
      ->setFormatConversionAllowed(true)
      ->setPerformanceMode(PerformanceMode::None)
      ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
      ->setDataCallback(this)
      ->setErrorCallback(this);

  auto result = builder.openStream(mStream_);

  if (result != oboe::Result::OK || mStream_ == nullptr) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Failed to open stream: %s",
        oboe::convertToText(result));
    return false;
  }

  streamSampleRate_ = mStream_->getSampleRate();
  streamChannelCount_ = mStream_->getChannelCount();
  streamMaxBufferSizeInFrames_ = mStream_->getBufferSizeInFrames();

  return true;
}

std::string AndroidAudioRecorder::start() {
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);

  if (isRecording()) {
    return "";
  }

  if (!openAudioStream()) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Couldn't open audio stream.\n");
    return "";
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return "";
  }

  if (usesFileOutput()) {
    filePath_ = fileWriter_->openFile(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (usesCallback()) {
    callback_->prepare(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  auto result = mStream_->requestStart();

  if (result != oboe::Result::OK) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Failed to start stream: %s",
        oboe::convertToText(result));
    return "";
  }

  jni::ThreadScope::WithClassLoader(
      [this]() { nativeAudioRecorder_->start(); });

  state_.store(RecorderState::Recording);
  return std::format("file://{}", filePath_);
}

std::tuple<std::string, double, double> AndroidAudioRecorder::stop() {
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);

  std::string filePath = std::format("file://{}", filePath_);
  double outputFileSize = 0.0;
  double outputDuration = 0.0;

  if (!isRecording()) {
    return {filePath, 0.0, 0.0};
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return {filePath, 0.0, 0.0};
  }

  jni::ThreadScope::WithClassLoader([this]() { nativeAudioRecorder_->stop(); });
  mStream_->requestStop();

  if (usesFileOutput()) {
    auto [size, duration] = fileWriter_->closeFile();
    outputFileSize = size;
    outputDuration = duration;
  }

  if (usesCallback()) {
    callback_->cleanup();
  }

  filePath_ = "";
  state_.store(RecorderState::Idle);
  return {filePath, outputFileSize, outputDuration};
}

void AndroidAudioRecorder::enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags) {
  Locker fileWriterLock(fileWriterMutex_);

  uint8_t format = static_cast<uint8_t>(androidFlags & 0xF);

  if (format == 1) {
    fileWriter_ = std::make_shared<MiniAudioFileWriter>(
        sampleRate, channelCount, bitRate, androidFlags);
  } else {
    fileWriter_ = std::make_shared<FFmpegAudioFileWriter>(
        sampleRate, channelCount, bitRate, androidFlags);
  }

  if (!isIdle()) {
    fileWriter_->openFile(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  fileOutputEnabled_.store(true);
}

void AndroidAudioRecorder::disableFileOutput() {
  Locker fileWriterLock(fileWriterMutex_);
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
  Locker callbackLock(callbackMutex_);
  callback_ = std::make_shared<AndroidRecorderCallback>(
      audioEventHandlerRegistry_,
      sampleRate,
      bufferLength,
      channelCount,
      callbackId);

  if (!isIdle()) {
    callback_->prepare(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  callbackOutputEnabled_.store(true);
}

void AndroidAudioRecorder::clearOnAudioReadyCallback() {
  Locker callbackLock(callbackMutex_);
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
    if (auto fileWriterLock = Locker::tryLock(fileWriterMutex_)) {
      fileWriter_->writeAudioData(audioData, numFrames);
    }
  }

  if (usesCallback()) {
    if (auto callbackLock = Locker::tryLock(callbackMutex_)) {
      callback_->receiveAudioData(audioData, numFrames);
    }
  }

  return DataCallbackResult::Continue;
}

double AndroidAudioRecorder::getCurrentDuration() const {
  if (usesFileOutput()) {
    return fileWriter_->getCurrentDuration();
  }

  return 0.0;
}

bool AndroidAudioRecorder::isRecording() const {
  return state_.load() == RecorderState::Recording &&
      mStream_->getState() == oboe::StreamState::Started;
}

bool AndroidAudioRecorder::isPaused() const {
  return state_.load() == RecorderState::Paused;
}

bool AndroidAudioRecorder::isIdle() const {
  return state_.load() == RecorderState::Idle;
}

void AndroidAudioRecorder::cleanup() {
  state_.store(RecorderState::Idle);

  if (mStream_) {
    mStream_->close();
    mStream_.reset();
  }
}

void AndroidAudioRecorder::onErrorAfterClose(
    oboe::AudioStream *stream,
    oboe::Result error) {
  if (error == oboe::Result::ErrorDisconnected) {
    cleanup();
    if (openAudioStream()) {
      mStream_->requestStart();
      state_.store(RecorderState::Recording);
    }
  }
}

} // namespace audioapi
