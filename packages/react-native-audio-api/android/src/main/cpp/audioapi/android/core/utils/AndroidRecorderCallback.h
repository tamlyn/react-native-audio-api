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
      int channelCount,
      uint64_t callbackId);
  ~AndroidRecorderCallback();

  void prepare(float streamSampleRate, int streamChannelCount, size_t maxInputBufferLength);
  void cleanup();

  void receiveAudioData(void *data, int numFrames);
  void emitAudioData(bool flush = false);

  void invokeCallback(const std::shared_ptr<AudioBus> &bus, int numFrames);

 private:
  float streamSampleRate_;
  int streamChannelCount_;
  size_t maxInputBufferLength_;

  float sampleRate_;
  int channelCount_;
  size_t bufferLength_;
  uint64_t callbackId_;
  size_t ringBufferSize_;

  ma_uint64 processingBufferLength_{0};
  void *processingBuffer_{nullptr};

  std::unique_ptr<ma_data_converter> converter_{nullptr};

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
  // TODO: CircularAudioBus
  std::vector<std::shared_ptr<CircularAudioArray>> circularBus_;
  std::shared_ptr<AudioArray> deinterleavingArray_;

  void deinterleaveAndPushAudioData(void *data, int numFrames);
};

} // namespace audioapi
