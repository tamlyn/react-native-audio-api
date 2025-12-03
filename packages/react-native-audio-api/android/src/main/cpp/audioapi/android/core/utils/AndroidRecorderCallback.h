#pragma once


#include <audioapi/libs/miniaudio/miniaudio.h>
#include <memory>
#include <vector>

namespace audioapi {

class AudioBus;
class AudioArray;
class CircularAudioArray;
class AudioEventHandlerRegistry;

class AndroidRecorderCallback {
 public:
  AndroidRecorderCallback(
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry,
      float sampleRate,
      size_t bufferLength,
      size_t channelCount,
      uint64_t callbackId);
  ~AndroidRecorderCallback();

  void prepare(int32_t streamSampleRate, int32_t streamChannelCount, size_t maxInputBufferLength);
  void cleanup();

  void receiveAudioData(void *data, int numFrames);
  void emitAudioData();

  void invokeCallback(const std::shared_ptr<AudioBus> &bus, int numFrames);
  void sendRemainingData();

 private:
  int32_t streamSampleRate_;
  int32_t streamChannelCount_;
  size_t maxInputBufferLength_;

  float sampleRate_;
  size_t bufferLength_;
  size_t channelCount_;
  uint64_t callbackId_;
  size_t ringBufferSize_;

  ma_uint64 processingBufferLength_{0};
  void *processingBuffer_{nullptr};

  std::unique_ptr<ma_data_converter> converter_{nullptr};

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
  // TODO: CircularAudioBus
  std::vector<std::shared_ptr<CircularAudioArray>> circularBus_;
  std::shared_ptr<AudioArray> deinterleavingArray_;

  void deinterleaveAndWriteAudioData(void *data, int numFrames);
};

} // namespace audioapi
