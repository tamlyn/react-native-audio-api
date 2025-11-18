#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <unordered_map>

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
#include <audioapi/utils/AudioFileProperties.hpp>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

IOSAudioRecorder::IOSAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
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

  size_t maxInputBufferLength = [nativeRecorder_ getBufferSize];

  if (isRecording()) {
    return ReturnStatus<std::string>::Error("Already recording");
  }

  if (usesFileOutput()) {
    auto result = fileWriter_->openFile([nativeRecorder_ getInputFormat], maxInputBufferLength);

    if (!result.isSuccess()) {
      return ReturnStatus<std::string>::Error("Failed to open file for writing: " + result.getMessage());
    }

    filePath_ = result.getValue();
  }

  if (usesCallback()) {
    callback_->prepare([nativeRecorder_ getInputFormat], maxInputBufferLength);
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
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

  [nativeRecorder_ stop];
  state_.store(RecorderState::Idle);

  if (usesFileOutput()) {
    auto result = fileWriter_->closeFile();

    if (!result.isSuccess()) {
      return ReturnStatus<std::tuple<std::string, double, double>>::Error(
          "Failed to close file: " + result.getMessage());
    }

    outputFileSize = std::get<0>(result.getValue());
    outputDuration = std::get<1>(result.getValue());
  }

  if (usesCallback()) {
    callback_->cleanup();
  }

  filePath_ = "";
  return ReturnStatus<std::tuple<std::string, double, double>>::Success(
      std::make_tuple(filePath, outputFileSize, outputDuration));
}

void IOSAudioRecorder::enableFileOutput(std::shared_ptr<AudioFileProperties> properties)
{
  Locker lock(fileWriterMutex_);

  fileWriter_ = std::make_shared<FileWriter>(properties);
  fileOutputEnabled_.store(true);
}

void IOSAudioRecorder::disableFileOutput()
{
  Locker lock(fileWriterMutex_);
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
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

  return currentState == RecorderState::Paused && [audioEngine getState] != AudioEngineState::AudioEngineStateRunning;
}

bool IOSAudioRecorder::isIdle() const
{
  return state_.load() == RecorderState::Idle;
}

void IOSAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId)
{
  Locker lock(callbackMutex_);

  callback_ = std::make_shared<RecorderCallback>(
      audioEventHandlerRegistry_, sampleRate, bufferLength, channelCount, callbackId);

  if (!isIdle()) {
    callback_->prepare([nativeRecorder_ getInputFormat], [nativeRecorder_ getBufferSize]);
  }

  callbackOutputEnabled_.store(true);
}

void IOSAudioRecorder::clearOnAudioReadyCallback()
{
  Locker lock(callbackMutex_);
  callbackOutputEnabled_.store(false);
  callback_ = nullptr;
}

void IOSAudioRecorder::setOnErrorCallback(uint64_t callbackId)
{
  errorCallbackId_.store(callbackId);
}

void IOSAudioRecorder::clearOnErrorCallback()
{
  errorCallbackId_.store(0);
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
