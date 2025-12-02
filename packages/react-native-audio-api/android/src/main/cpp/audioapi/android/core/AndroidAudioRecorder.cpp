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
#include <audioapi/utils/AudioFileProperties.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

#include <memory>
#include <string>

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

ReturnStatus<void> AndroidAudioRecorder::openAudioStream() {
  if (mStream_) {
    return ReturnStatus<void>::Success();
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
    return ReturnStatus<void>::Error(
        "Failed to open audio stream: " + std::string(oboe::convertToText(result)));
  }

  streamSampleRate_ = mStream_->getSampleRate();
  streamChannelCount_ = mStream_->getChannelCount();
  streamMaxBufferSizeInFrames_ = mStream_->getBufferSizeInFrames();

  return ReturnStatus<void>::Success();
}

ReturnStatus<std::string> AndroidAudioRecorder::start() {
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);

  if (isRecording()) {
    return ReturnStatus<std::string>::Success(std::format("file://{}", filePath_));
  }

  auto streamResult = openAudioStream();

  if (!streamResult.isSuccess()) {
    return ReturnStatus<std::string>::Error(streamResult.getMessage());
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    return ReturnStatus<std::string>::Error("Audio stream is not initialized.");
  }

  if (usesFileOutput()) {
    auto fileResult =
        fileWriter_->openFile(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);

    if (!fileResult.isSuccess()) {
      return ReturnStatus<std::string>::Error(
          "Failed to open file for writing: " + fileResult.getMessage());
    }

    filePath_ = fileResult.getValue();
  }

  if (usesCallback()) {
    callback_->prepare(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  auto result = mStream_->requestStart();

  if (result != oboe::Result::OK) {
    return ReturnStatus<std::string>::Error(
        "Failed to start stream: " + std::string(oboe::convertToText(result)));
  }

  jni::ThreadScope::WithClassLoader([this]() { nativeAudioRecorder_->start(); });

  state_.store(RecorderState::Recording);
  return ReturnStatus<std::string>::Success(std::format("file://{}", filePath_));
}

ReturnStatus<std::tuple<std::string, double, double>> AndroidAudioRecorder::stop() {
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);

  std::string filePath = std::format("file://{}", filePath_);
  double outputFileSize = 0.0;
  double outputDuration = 0.0;

  if (!isRecording()) {
    return ReturnStatus<std::tuple<std::string, double, double>>::Error(
        "Recorder is not in recording state.");
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    return ReturnStatus<std::tuple<std::string, double, double>>::Error(
        "Audio stream is not initialized.");
  }

  state_.store(RecorderState::Idle);
  jni::ThreadScope::WithClassLoader([this]() { nativeAudioRecorder_->stop(); });
  mStream_->requestStop();

  if (usesFileOutput()) {
    auto fileResult = fileWriter_->closeFile();

    if (!fileResult.isSuccess()) {
      return ReturnStatus<std::tuple<std::string, double, double>>::Error(
          "Failed to close file: " + fileResult.getMessage());
    }

    outputFileSize = std::get<0>(fileResult.getValue());
    outputDuration = std::get<1>(fileResult.getValue());
  }

  if (usesCallback()) {
    callback_->cleanup();
  }

  filePath_ = "";
  return ReturnStatus<std::tuple<std::string, double, double>>::Success(
      {filePath, outputFileSize, outputDuration});
}

ReturnStatus<std::string> AndroidAudioRecorder::enableFileOutput(
    std::shared_ptr<AudioFileProperties> properties) {
  Locker fileWriterLock(fileWriterMutex_);

  if (properties->format == AudioFileProperties::Format::WAV) {
    fileWriter_ = std::make_shared<MiniAudioFileWriter>(properties);
  } else {
    fileWriter_ = std::make_shared<android::ffmpeg::FFmpegAudioFileWriter>(properties);
  }

  if (!isIdle()) {
    auto fileResult =
        fileWriter_->openFile(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);

    if (!fileResult.isSuccess()) {
      return ReturnStatus<std::string>::Error(
          "Failed to open file for writing: " + fileResult.getMessage());
    }

    filePath_ = fileResult.getValue();
  }

  fileOutputEnabled_.store(true);
  return ReturnStatus<std::string>::Success(filePath_);
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

ReturnStatus<void> AndroidAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId) {
  Locker callbackLock(callbackMutex_);
  callback_ = std::make_shared<AndroidRecorderCallback>(
      audioEventHandlerRegistry_, sampleRate, bufferLength, channelCount, callbackId);

  if (!isIdle()) {
    callback_->prepare(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  callbackOutputEnabled_.store(true);

  return ReturnStatus<void>::Success();
}

void AndroidAudioRecorder::clearOnAudioReadyCallback() {
  Locker callbackLock(callbackMutex_);
  callbackOutputEnabled_.store(false);
  callback_ = nullptr;
}

void AndroidAudioRecorder::setOnErrorCallback(uint64_t callbackId) {
  // audioEventHandlerRegistry_->registerRecorderErrorCallback(callbackId);
}

void AndroidAudioRecorder::clearOnErrorCallback() {
  // audioEventHandlerRegistry_->unregisterRecorderErrorCallback();
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

void AndroidAudioRecorder::onErrorAfterClose(oboe::AudioStream *stream, oboe::Result error) {
  if (error == oboe::Result::ErrorDisconnected) {
    cleanup();

    auto streamResult = openAudioStream();

    if (!streamResult.isSuccess()) {
      // TODO: call error callback
      return;
    }

    mStream_->requestStart();
    state_.store(RecorderState::Recording);
  }
}

} // namespace audioapi
