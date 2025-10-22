#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/core/utils/Constants.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/IOSAudioRecorder.h>
#include <audioapi/ios/core/IOSAudioFileWriter.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>
#include <unordered_map>

namespace audioapi {

IOSAudioRecorder::IOSAudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry), fileWriter_(nullptr) {
}

IOSAudioRecorder::~IOSAudioRecorder() {
  // stop();
  // [audioRecorder_ cleanup]; --- IGNORE ---
}

void IOSAudioRecorder::start() { }

std::string IOSAudioRecorder::stop() {
  return std::string("");
}

void IOSAudioRecorder::enableFileOutput(
        float sampleRate,
      size_t channelCount,
      size_t bitRate,
      size_t iosFlags,
      size_t androidFlags) {
  fileOutputEnabled_.store(true);
  fileWriter_ = std::make_shared<IOSAudioFileWriter>(
    sampleRate,
    channelCount,
    bitRate,
    iosFlags);
}

void IOSAudioRecorder::disableFileOutput() {
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
}

void IOSAudioRecorder::pause() {

}

void IOSAudioRecorder::resume() {

}
// IOSAudioRecorder::IOSAudioRecorder(
//     float sampleRate,
//     int bufferLength,
//     bool recordToFile,
//     const std::string &fileDirectory,
//     const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
//     : AudioRecorder(sampleRate, bufferLength, recordToFile, fileDirectory, audioEventHandlerRegistry)
// {
//   currentFileURL_ = nil;
//   currentAudioFile_ = nil;

//   AudioReceiverBlock audioReceiverBlock = ^(const AudioBufferList *inputBuffer, int numFrames) {
//     if (isRunning_.load()) {
//       auto *inputChannel = static_cast<float *>(inputBuffer->mBuffers[0].mData);
//       writeToBuffers(inputChannel, numFrames);
//     }

//     while (circularBuffer_->getNumberOfAvailableFrames() >= bufferLength_) {
//       auto bus = std::make_shared<AudioBus>(bufferLength_, 1, sampleRate_);
//       auto *outputChannel = bus->getChannel(0)->getData();

//       circularBuffer_->pop_front(outputChannel, bufferLength_);

//       invokeOnAudioReadyCallback(bus, bufferLength_);
//     }

//     if (recordToFile_) {
//       writeToFile(inputBuffer, numFrames);
//     }
//   };

//   audioRecorder_ = [[NativeAudioRecorder alloc] initWithReceiverBlock:audioReceiverBlock
//                                                          bufferLength:bufferLength
//                                                            sampleRate:sampleRate];
// }

// IOSAudioRecorder::~IOSAudioRecorder()
// {
//   stop();
//   [audioRecorder_ cleanup];
// }

// void IOSAudioRecorder::start()
// {
//   if (isRunning_.load()) {
//     return;
//   }

//   if (recordToFile_) {
//     createFileForWriting();
//   }

//   [audioRecorder_ start];
//   isRunning_.store(true);
// }

// void IOSAudioRecorder::stop()
// {
//   if (!isRunning_.load()) {
//     return;
//   }

//   isRunning_.store(false);
//   [audioRecorder_ stop];

//   sendRemainingData();

//   if (recordToFile_) {
//     releaseFile();
//   }
// }

} // namespace audioapi
