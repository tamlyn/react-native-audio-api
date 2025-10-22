#pragma once

#include <memory>
#include <atomic>
#include <mutex>
#include <string>

namespace audioapi {

class RecorderAdapterNode;
class AudioBus;
class CircularAudioArray;
class AudioEventHandlerRegistry;

class AudioRecorder {
 public:
  explicit AudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  virtual ~AudioRecorder() = default;

  virtual void start() = 0;
  virtual std::string stop() = 0;

  bool isRecording();

  virtual void enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags) = 0;
  virtual void disableFileOutput() = 0;

  virtual void pause() = 0;
  virtual void resume() = 0;

  void connect(const std::shared_ptr<RecorderAdapterNode> &node);
  void disconnect();

  void setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId);
  void clearOnAudioReadyCallback();

  void invokeOnAudioReadyCallback(
      const std::shared_ptr<AudioBus> &bus,
      int numFrames);
  void sendRemainingCallbackData();

  bool usesCallback() const;
  bool usesFileOutput() const;
  bool isConnected() const;

 protected:
  struct CallbackProperties {
    float sampleRate;
    size_t bufferLength;
    size_t channelCount;
    uint64_t callbackId;
  };

  CallbackProperties callbackProperties_;
  // size_t ringBufferSize_;

  std::atomic<bool> isRunning_;
  std::atomic<bool> fileOutputEnabled_;
  std::atomic<bool> callbackOutputEnabled_;
  std::atomic<bool> isConnected_;
  // std::shared_ptr<CircularAudioArray> circularBuffer_;

  mutable std::mutex adapterNodeLock_;
  std::shared_ptr<RecorderAdapterNode> adapterNode_ = nullptr;

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
  uint64_t onAudioReadyCallbackId_ = 0;

  // void writeToBuffers(const float *data, int numFrames);
};

} // namespace audioapi
