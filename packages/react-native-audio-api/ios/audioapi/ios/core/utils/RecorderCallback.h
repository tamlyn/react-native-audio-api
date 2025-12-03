#pragma once

#ifndef __OBJC__ // when compiled as C++
typedef struct objc_object AVAudioFormat;
typedef struct objc_object AudioBufferList;
typedef struct objc_object AVAudioConverter;
#endif

#include <audioapi/utils/ReturnStatus.hpp>
#include <memory>
#include <vector>

namespace audioapi {

class AudioBus;
class CircularAudioArray;
class AudioEventHandlerRegistry;

class RecorderCallback {
 public:
  RecorderCallback(
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry,
      float sampleRate,
      size_t bufferLength,
      size_t channelCount,
      uint64_t callbackId);
  ~RecorderCallback();

  ReturnStatus<void> prepare(AVAudioFormat *bufferFormat, size_t maxInputBufferLength);
  void cleanup();

  void receiveAudioData(const AudioBufferList *audioBufferList, int numFrames);
  void emitAudioData(bool flush = false);

  void invokeCallback(const std::shared_ptr<AudioBus> &bus, int numFrames);

  void setOnErrorCallback(uint64_t callbackId);
  void clearOnErrorCallback();
  void invokeOnErrorCallback(const std::string &message);

 private:
  std::atomic<bool> isInitialized_{false};

  float sampleRate_;
  size_t bufferLength_;
  size_t channelCount_;
  uint64_t callbackId_;
  size_t ringBufferSize_;
  size_t converterInputBufferSize_;
  size_t converterOutputBufferSize_;

  std::atomic<uint64_t> errorCallbackId_{0};

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
  // TODO: CircularAudioBus
  std::vector<std::shared_ptr<CircularAudioArray>> circularBus_;

  AVAudioFormat *bufferFormat_;
  AVAudioFormat *callbackFormat_;
  AVAudioConverter *converter_;

  AVAudioPCMBuffer *converterInputBuffer_;
  AVAudioPCMBuffer *converterOutputBuffer_;
};

} // namespace audioapi
