#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <unordered_map>

#include <audioapi/core/utils/Constants.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/IOSAudioFileWriter.h>
#include <audioapi/ios/core/IOSAudioRecorder.h>
#include <audioapi/ios/core/IOSRecorderCallback.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

// AudioReceiverBlock audioReceiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
//   if (isRunning_.load()) {
//     auto *inputChannel = static_cast<float *>(inputBuffer->mBuffers[0].mData);
//     writeToBuffers(inputChannel, numFrames);
//   }
// };

IOSAudioRecorder::IOSAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry), fileWriter_(nullptr)
{
  AudioReceiverBlock receiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
    if (usesFileOutput()) {
      fileWriter_->writeAudioData(inputBuffer, numFrames);
    }

    if (usesCallback()) {
      callback_->receiveAudioData(inputBuffer, numFrames);
    }
  };

  nativeRecorder_ = [[NativeAudioRecorder alloc] initWithReceiverBlock:receiverBlock];
}

IOSAudioRecorder::~IOSAudioRecorder()
{
  stop();
  [nativeRecorder_ cleanup];
}
std::string IOSAudioRecorder::start()
{
  size_t maxInputBufferLength = [nativeRecorder_ getBufferSize];

  if (isRecording()) {
    return filePath_;
  }

  if (usesFileOutput()) {
    filePath_ = fileWriter_->openFile([nativeRecorder_ getInputFormat]);
  }

  if (usesCallback()) {
    callback_->prepare([nativeRecorder_ getInputFormat], maxInputBufferLength);
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  [nativeRecorder_ start];
  isRunning_.store(true);

  return filePath_;
}

std::tuple<std::string, double, double> IOSAudioRecorder::stop()
{
  std::string filePath = filePath_;
  double outputFileSize = 0;
  double outputDuration = 0;

  if (!isRecording()) {
    return {filePath, 0, 0};
  }

  [nativeRecorder_ stop];
  isRunning_.store(false);

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

void IOSAudioRecorder::enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags)
{
  fileOutputEnabled_.store(true);
  fileWriter_ = std::make_shared<IOSAudioFileWriter>(sampleRate, channelCount, bitRate, iosFlags);
}

void IOSAudioRecorder::disableFileOutput()
{
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
}

void IOSAudioRecorder::pause()
{
  [nativeRecorder_ stop];
  isRunning_.store(false);
}

void IOSAudioRecorder::resume()
{
  [nativeRecorder_ start];
  isRunning_.store(true);
}

void IOSAudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId)
{
  callback_ = std::make_shared<IOSRecorderCallback>(
      audioEventHandlerRegistry_, sampleRate, bufferLength, channelCount, callbackId);
  callbackOutputEnabled_.store(true);
}

void IOSAudioRecorder::clearOnAudioReadyCallback()
{
  callbackOutputEnabled_.store(false);
  callback_ = nullptr;
}

} // namespace audioapi
