#import <AVFoundation/AVFoundation.h>
#import <AudioEngine.h>
#import <AudioSessionManager.h>
#import <Foundation/Foundation.h>

#include <unordered_map>

#include <audioapi/core/sources/RecorderAdapterNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/core/utils/Locker.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/IOSAudioRecorder.h>
#include <audioapi/ios/core/utils/FileWriter.h>
#include <audioapi/ios/core/utils/RecorderCallback.h>
#include <audioapi/ios/system/AudioEngine.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/AudioFileProperties.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

IOSAudioRecorder::IOSAudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry), fileWriter_(nullptr)
{
  AudioReceiverBlock receiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
    if (usesFileOutput()) {
      if (auto lock = Locker::tryLock(fileWriterMutex_)) {
        fileWriter_->writeAudioData(inputBuffer, numFrames);
      }
    }

    if (usesCallback()) {
      if (auto lock = Locker::tryLock(callbackMutex_)) {
        callback_->receiveAudioData(inputBuffer, numFrames);
      }
    }

    if (isConnected()) {
      if (auto lock = Locker::tryLock(adapterNodeMutex_)) {
        for (size_t channel = 0; channel < adapterNode_->channelCount_; ++channel) {
          float *channelData = (float *)inputBuffer->mBuffers[channel].mData;

          adapterNode_->buff_[channel]->write(channelData, numFrames);
        }
      }
    }
  };

  nativeRecorder_ = [[NativeAudioRecorder alloc] initWithReceiverBlock:receiverBlock];
}

IOSAudioRecorder::~IOSAudioRecorder()
{
  stop();
  [nativeRecorder_ cleanup];
}

ReturnStatus<std::string> IOSAudioRecorder::start()
{
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);
  Locker adapterLock(adapterNodeMutex_);
  AudioSessionManager *audioSessionManager = [AudioSessionManager sharedInstance];

  if ([[audioSessionManager checkRecordingPermissions] isEqual:@"Denied"]) {
    return ReturnStatus<std::string>::Error("Microphone permissions are not granted");
  }

  // TODO: recorder should probably request activating the session and setting the options if not set by user
  // but lets handle that in another PR
  if (![audioSessionManager isSessionActive]) {
    return ReturnStatus<std::string>::Error("Audio session is not active");
  }

  size_t maxInputBufferLength = [nativeRecorder_ getBufferSize];
  auto inputFormat = [nativeRecorder_ getInputFormat];

  NSLog(@"Starting IOSAudioRecorder with input format: %@", inputFormat);

  if (isRecording()) {
    return ReturnStatus<std::string>::Error("Already recording");
  }

  if (usesFileOutput()) {
    auto fileResult = fileWriter_->openFile(inputFormat, maxInputBufferLength);

    if (!fileResult.isSuccess()) {
      return ReturnStatus<std::string>::Error(
          "Failed to open file for writing: " + fileResult.getMessage());
    }

    filePath_ = fileResult.getValue();
  }

  if (usesCallback()) {
    auto callbackResult = callback_->prepare(inputFormat, maxInputBufferLength);

    if (!callbackResult.isSuccess()) {
      return ReturnStatus<std::string>::Error(
          "Failed to prepare callback: " + callbackResult.getMessage());
    }
  }

  if (isConnected()) {
    adapterNode_->init(maxInputBufferLength, inputFormat.channelCount);
  }

  [nativeRecorder_ start];
  state_.store(RecorderState::Recording);

  return ReturnStatus<std::string>::Success(filePath_);
}

ReturnStatus<std::tuple<std::string, double, double>> IOSAudioRecorder::stop()
{
  Locker callbackLock(callbackMutex_);
  Locker fileWriterLock(fileWriterMutex_);

  std::string filePath = filePath_;
  double outputFileSize = 0;
  double outputDuration = 0;

  if (!isRecording()) {
    return ReturnStatus<std::tuple<std::string, double, double>>::Error("Not recording");
  }

  state_.store(RecorderState::Idle);
  [nativeRecorder_ stop];

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
      std::make_tuple(filePath, outputFileSize, outputDuration));
}

ReturnStatus<std::string> IOSAudioRecorder::enableFileOutput(
    std::shared_ptr<AudioFileProperties> properties)
{
  Locker lock(fileWriterMutex_);
  fileWriter_ = std::make_shared<FileWriter>(audioEventHandlerRegistry_, properties);

  if (!isIdle()) {
    auto result =
        fileWriter_->openFile([nativeRecorder_ getInputFormat], [nativeRecorder_ getBufferSize]);

    if (!result.isSuccess()) {
      return ReturnStatus<std::string>::Error(
          "Failed to open file for writing: " + result.getMessage());
    }

    filePath_ = result.getValue();
  }

  // TODO: atomic szpont?
  if (errorCallbackId_.load() != 0) {
    fileWriter_->setOnErrorCallback(errorCallbackId_.load());
  }

  fileOutputEnabled_.store(true);
  return ReturnStatus<std::string>::Success(filePath_);
}

void IOSAudioRecorder::disableFileOutput()
{
  Locker lock(fileWriterMutex_);
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
}

void IOSAudioRecorder::connect(const std::shared_ptr<RecorderAdapterNode> &node)
{
  Locker lock(adapterNodeMutex_);
  adapterNode_ = node;

  if (!isIdle()) {
    adapterNode_->init(
        [nativeRecorder_ getBufferSize], [nativeRecorder_ getInputFormat].channelCount);
  }

  isConnected_.store(true);
}

void IOSAudioRecorder::disconnect()
{
  Locker lock(adapterNodeMutex_);
  adapterNode_ = nullptr;
  isConnected_.store(false);
}

void IOSAudioRecorder::pause()
{
  if (!isRecording()) {
    return;
  }

  [nativeRecorder_ pause];
  state_.store(RecorderState::Paused);
}

void IOSAudioRecorder::resume()
{
  if (!isPaused()) {
    return;
  }

  [nativeRecorder_ resume];
  state_.store(RecorderState::Recording);
}

bool IOSAudioRecorder::isRecording() const
{
  AudioEngine *audioEngine = [AudioEngine sharedInstance];
  return state_.load() == RecorderState::Recording &&
      [audioEngine getState] == AudioEngineState::AudioEngineStateRunning;
}

bool IOSAudioRecorder::isPaused() const
{
  AudioEngine *audioEngine = [AudioEngine sharedInstance];
  auto currentState = state_.load();

  if (currentState == RecorderState::Idle) {
    return false;
  }

  return currentState == RecorderState::Paused &&
      [audioEngine getState] != AudioEngineState::AudioEngineStateRunning;
}

bool IOSAudioRecorder::isIdle() const
{
  return state_.load() == RecorderState::Idle;
}

ReturnStatus<void> IOSAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId)
{
  Locker lock(callbackMutex_);

  callback_ = std::make_shared<RecorderCallback>(
      audioEventHandlerRegistry_, sampleRate, bufferLength, channelCount, callbackId);

  if (!isIdle()) {
    auto result =
        callback_->prepare([nativeRecorder_ getInputFormat], [nativeRecorder_ getBufferSize]);

    if (!result.isSuccess()) {
      return ReturnStatus<void>::Error(result.getMessage());
    }
  }

  if (errorCallbackId_.load() != 0) {
    callback_->setOnErrorCallback(errorCallbackId_.load());
  }

  callbackOutputEnabled_.store(true);
  return ReturnStatus<void>::Success();
}

void IOSAudioRecorder::clearOnAudioReadyCallback()
{
  Locker lock(callbackMutex_);
  callbackOutputEnabled_.store(false);
  callback_ = nullptr;
}

void IOSAudioRecorder::setOnErrorCallback(uint64_t callbackId)
{
  if (usesFileOutput()) {
    fileWriter_->setOnErrorCallback(callbackId);
  }

  if (usesCallback()) {
    callback_->setOnErrorCallback(callbackId);
  }
  errorCallbackId_.store(callbackId);
}

void IOSAudioRecorder::clearOnErrorCallback()
{
  errorCallbackId_.store(0);

  if (usesFileOutput()) {
    fileWriter_->clearOnErrorCallback();
  }

  if (usesCallback()) {
    callback_->clearOnErrorCallback();
  }
}

double IOSAudioRecorder::getCurrentDuration() const
{
  double duration = 0.0;

  if (usesFileOutput() && fileWriter_) {
    duration = fileWriter_->getCurrentDuration();
  }

  return duration;
}

} // namespace audioapi
