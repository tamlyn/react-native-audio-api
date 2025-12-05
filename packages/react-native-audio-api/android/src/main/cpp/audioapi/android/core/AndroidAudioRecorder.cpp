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
    : AudioRecorder(audioEventHandlerRegistry),
      streamSampleRate_(0.0),
      streamChannelCount_(0),
      streamMaxBufferSizeInFrames_(0) {
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

Result<NoneType, std::string> AndroidAudioRecorder::openAudioStream() {
  if (mStream_) {
    return Result<NoneType, std::string>::Ok(None);
  }

  oboe::AudioStreamBuilder builder;
  builder.setSharingMode(oboe::SharingMode::Exclusive)
      ->setDirection(oboe::Direction::Input)
      ->setFormat(oboe::AudioFormat::Float)
      ->setFormatConversionAllowed(true)
      ->setPerformanceMode(oboe::PerformanceMode::None)
      ->setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Medium)
      ->setDataCallback(this)
      ->setErrorCallback(this);

  auto result = builder.openStream(mStream_);

  if (result != oboe::Result::OK || mStream_ == nullptr) {
    return Result<NoneType, std::string>::Err(
        "Failed to open audio stream: " + std::string(oboe::convertToText(result)));
  }

  streamSampleRate_ = static_cast<float>(mStream_->getSampleRate());
  streamChannelCount_ = mStream_->getChannelCount();
  streamMaxBufferSizeInFrames_ = mStream_->getBufferSizeInFrames();

  return Result<NoneType, std::string>::Ok(None);
}

Result<std::string, std::string> AndroidAudioRecorder::start() {
  std::scoped_lock startLock(callbackMutex_, fileWriterMutex_, adapterNodeMutex_);

  if (isRecording()) {
    return Result<std::string, std::string>::Ok(std::format("file://{}", filePath_));
  }

  auto streamResult = openAudioStream();

  if (!streamResult.is_ok()) {
    return Result<std::string, std::string>::Err(streamResult.unwrap_err());
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    return Result<std::string, std::string>::Err("Audio stream is not initialized.");
  }

  if (usesFileOutput()) {
    auto fileResult =
        std::dynamic_pointer_cast<AndroidFileWriterBackend>(fileWriter_)
            ->openFile(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);

    if (!fileResult.is_ok()) {
      return Result<std::string, std::string>::Err(
          "Failed to open file for writing: " + fileResult.unwrap_err());
    }

    filePath_ = fileResult.unwrap();
  }

  if (usesCallback()) {
    std::dynamic_pointer_cast<AndroidRecorderCallback>(dataCallback_)
        ->prepare(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (isConnected()) {
    adapterNode_->init(streamMaxBufferSizeInFrames_, streamChannelCount_);
  }

  auto result = mStream_->requestStart();

  if (result != oboe::Result::OK) {
    return Result<std::string, std::string>::Err(
        "Failed to start stream: " + std::string(oboe::convertToText(result)));
  }

  jni::ThreadScope::WithClassLoader([this]() { nativeAudioRecorder_->start(); });

  state_.store(RecorderState::Recording, std::memory_order_release);
  return Result<std::string, std::string>::Ok(std::format("file://{}", filePath_));
}

Result<std::tuple<std::string, double, double>, std::string> AndroidAudioRecorder::stop() {
  std::scoped_lock stopLock(callbackMutex_, fileWriterMutex_, adapterNodeMutex_);

  std::string filePath = std::format("file://{}", filePath_);
  double outputFileSize = 0.0;
  double outputDuration = 0.0;

  if (!isRecording()) {
    return Result<std::tuple<std::string, double, double>, std::string>::Err(
        "Recorder is not in recording state.");
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    return Result<std::tuple<std::string, double, double>, std::string>::Err(
        "Audio stream is not initialized.");
  }

  state_.store(RecorderState::Idle, std::memory_order_release);
  jni::ThreadScope::WithClassLoader([this]() { nativeAudioRecorder_->stop(); });
  mStream_->requestStop();

  if (usesFileOutput()) {
    auto fileResult = fileWriter_->closeFile();

    if (!fileResult.is_ok()) {
      return Result<std::tuple<std::string, double, double>, std::string>::Err(
          "Failed to close file: " + fileResult.unwrap_err());
    }

    outputFileSize = std::get<0>(fileResult.unwrap());
    outputDuration = std::get<1>(fileResult.unwrap());
  }

  if (usesCallback()) {
    dataCallback_->cleanup();
  }

  if (isConnected()) {
    adapterNode_->cleanup();
  }

  filePath_ = "";
  return Result<std::tuple<std::string, double, double>, std::string>::Ok(
      {filePath, outputFileSize, outputDuration});
}

Result<std::string, std::string> AndroidAudioRecorder::enableFileOutput(
    std::shared_ptr<AudioFileProperties> properties) {
  std::scoped_lock fileWriterLock(fileWriterMutex_);

  if (properties->format == AudioFileProperties::Format::WAV) {
    fileWriter_ = std::make_shared<MiniAudioFileWriter>(audioEventHandlerRegistry_, properties);
  } else {
    fileWriter_ = std::make_shared<android::ffmpeg::FFmpegAudioFileWriter>(
        audioEventHandlerRegistry_, properties);
  }

  if (!isIdle()) {
    auto fileResult =
        std::dynamic_pointer_cast<AndroidFileWriterBackend>(fileWriter_)
            ->openFile(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);

    if (!fileResult.is_ok()) {
      return Result<std::string, std::string>::Err(
          "Failed to open file for writing: " + fileResult.unwrap_err());
    }

    filePath_ = fileResult.unwrap();
  }

  fileOutputEnabled_.store(true, std::memory_order_release);
  return Result<std::string, std::string>::Ok(filePath_);
}

void AndroidAudioRecorder::disableFileOutput() {
  std::scoped_lock fileWriterLock(fileWriterMutex_);
  fileOutputEnabled_.store(false, std::memory_order_release);
  fileWriter_ = nullptr;
}

void AndroidAudioRecorder::pause() {
  if (!isRecording()) {
    return;
  }

  mStream_->pause(0);
  state_.store(RecorderState::Paused, std::memory_order_release);
}

void AndroidAudioRecorder::resume() {
  if (!isPaused()) {
    return;
  }

  mStream_->start(0);
  state_.store(RecorderState::Recording, std::memory_order_release);
}

Result<NoneType, std::string> AndroidAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    int channelCount,
    uint64_t callbackId) {
  std::scoped_lock callbackLock(callbackMutex_);
  dataCallback_ = std::make_shared<AndroidRecorderCallback>(
      audioEventHandlerRegistry_, sampleRate, bufferLength, channelCount, callbackId);

  if (!isIdle()) {
    std::dynamic_pointer_cast<AndroidRecorderCallback>(dataCallback_)
        ->prepare(streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  callbackOutputEnabled_.store(true, std::memory_order_release);

  return Result<NoneType, std::string>::Ok(None);
}

void AndroidAudioRecorder::clearOnAudioReadyCallback() {
  std::scoped_lock callbackLock(callbackMutex_);
  callbackOutputEnabled_.store(false, std::memory_order_release);
  dataCallback_ = nullptr;
}

void AndroidAudioRecorder::connect(const std::shared_ptr<RecorderAdapterNode> &node) {
  std::scoped_lock adapterLock(adapterNodeMutex_);
  adapterNode_ = node;
  deinterleavingBuffer_ = std::make_shared<AudioArray>(streamMaxBufferSizeInFrames_);

  if (!isIdle()) {
    adapterNode_->init(streamMaxBufferSizeInFrames_, streamChannelCount_);
  }

  isConnected_.store(true, std::memory_order_release);
}

void AndroidAudioRecorder::disconnect() {
  std::scoped_lock adapterLock(adapterNodeMutex_);
  isConnected_.store(false, std::memory_order_release);
  deinterleavingBuffer_ = nullptr;
  adapterNode_ = nullptr;
}

oboe::DataCallbackResult AndroidAudioRecorder::onAudioReady(
    oboe::AudioStream *oboeStream,
    void *audioData,
    int32_t numFrames) {
  if (isPaused()) {
    return oboe::DataCallbackResult::Continue;
  }

  if (usesFileOutput()) {
    if (auto fileWriterLock = Locker::tryLock(fileWriterMutex_)) {
      std::dynamic_pointer_cast<AndroidFileWriterBackend>(fileWriter_)
          ->writeAudioData(audioData, numFrames);
    }
  }

  if (usesCallback()) {
    if (auto callbackLock = Locker::tryLock(callbackMutex_)) {
      std::dynamic_pointer_cast<AndroidRecorderCallback>(dataCallback_)
          ->receiveAudioData(audioData, numFrames);
    }
  }

  if (isConnected()) {
    if (auto adapterLock = Locker::tryLock(adapterNodeMutex_)) {
      for (int channel = 0; channel < streamChannelCount_; ++channel) {
        for (int frame = 0; frame < numFrames; ++frame) {
          deinterleavingBuffer_->getData()[frame] =
              static_cast<float *>(audioData)[frame * streamChannelCount_ + channel];
        }

        adapterNode_->buff_[channel]->write(deinterleavingBuffer_->getData(), numFrames);
      }
    }
  }

  return oboe::DataCallbackResult::Continue;
}

bool AndroidAudioRecorder::isRecording() const {
  return state_.load(std::memory_order_acquire) == RecorderState::Recording &&
      mStream_->getState() == oboe::StreamState::Started;
}

bool AndroidAudioRecorder::isPaused() const {
  return state_.load(std::memory_order_acquire) == RecorderState::Paused;
}

bool AndroidAudioRecorder::isIdle() const {
  return state_.load(std::memory_order_acquire) == RecorderState::Idle;
}

void AndroidAudioRecorder::cleanup() {
  state_.store(RecorderState::Idle, std::memory_order_release);

  if (mStream_) {
    mStream_->close();
    mStream_.reset();
  }
}

void AndroidAudioRecorder::onErrorAfterClose(oboe::AudioStream *stream, oboe::Result error) {
  if (error == oboe::Result::ErrorDisconnected) {
    cleanup();

    auto streamResult = openAudioStream();

    if (!streamResult.is_ok()) {
      // TODO: call error callback
      return;
    }

    mStream_->requestStart();
    state_.store(RecorderState::Recording, std::memory_order_release);
  }
}

} // namespace audioapi
