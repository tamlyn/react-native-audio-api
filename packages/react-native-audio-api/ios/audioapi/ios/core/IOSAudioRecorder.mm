#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/core/utils/Constants.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/IOSAudioFileWriter.h>
#include <audioapi/ios/core/IOSAudioRecorder.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>
#include <unordered_map>

namespace audioapi {

// AudioReceiverBlock audioReceiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
//   if (isRunning_.load()) {
//     auto *inputChannel = static_cast<float *>(inputBuffer->mBuffers[0].mData);
//     writeToBuffers(inputChannel, numFrames);
//   }

//   while (circularBuffer_->getNumberOfAvailableFrames() >= bufferLength_) {
//     auto bus = std::make_shared<AudioBus>(bufferLength_, 1, sampleRate_);
//     auto *outputChannel = bus->getChannel(0)->getData();

//     circularBuffer_->pop_front(outputChannel, bufferLength_);

//     invokeOnAudioReadyCallback(bus, bufferLength_);
//   }
// };

IOSAudioRecorder::IOSAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry), fileWriter_(nullptr)
{
  AudioReceiverBlock receiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
    if (usesFileOutput()) {
      fileWriter_->writeAudioData(inputBuffer, numFrames);
    }
  };

  nativeRecorder_ = [[NativeAudioRecorder alloc] initWithReceiverBlock:receiverBlock];
}

IOSAudioRecorder::~IOSAudioRecorder()
{
  stop();
  [nativeRecorder_ cleanup];
}

void IOSAudioRecorder::start()
{
  if (isRecording()) {
    return;
  }

  if (usesFileOutput()) {
    NSLog(@"input format: %@", [nativeRecorder_ getInputFormat]);
    fileWriter_->openFile([nativeRecorder_ getInputFormat]);
  }

  if (usesCallback()) {
    // TODO: create circular buffer and converter?
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  [nativeRecorder_ start];
  isRunning_.store(true);
}

std::string IOSAudioRecorder::stop()
{
  if (!isRecording()) {
    return std::string("");
  }

  isRunning_.store(false);
  [nativeRecorder_ stop];

  // TODO: send remaining data?

  if (usesFileOutput()) {
    return fileWriter_->closeFile();
  }

  return std::string("");
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

void IOSAudioRecorder::pause() {}

void IOSAudioRecorder::resume() {}

} // namespace audioapi
