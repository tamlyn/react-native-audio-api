#pragma once

#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <tuple>

namespace audioapi {

class RecorderAdapterNode;
class AudioBus;
class CircularAudioArray;
class AudioEventHandlerRegistry;

class AudioRecorder {
 public:
  explicit AudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry):
      audioEventHandlerRegistry_(audioEventHandlerRegistry) {}
  virtual ~AudioRecorder() = default;

  virtual std::string start() = 0;
  virtual std::tuple<std::string, double, double> stop() = 0;

  virtual void enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags) = 0;
  virtual void disableFileOutput() = 0;

  virtual void pause() = 0;
  virtual void resume() = 0;

  void connect(const std::shared_ptr<RecorderAdapterNode> &node) {}
  void disconnect() {}

  virtual void setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId) = 0;
  virtual void clearOnAudioReadyCallback() = 0;

  bool usesCallback() const {
    return callbackOutputEnabled_.load();
  }

  bool usesFileOutput() const {
    return fileOutputEnabled_.load();
  }

  bool isConnected() const {
    return isConnected_.load();
  }

  bool isRecording() const {
    return isRunning_.load();
  }

 protected:
  // size_t ringBufferSize_;

  std::atomic<bool> isRunning_{false};
  std::atomic<bool> fileOutputEnabled_{false};
  std::atomic<bool> callbackOutputEnabled_{false};
  std::atomic<bool> isConnected_{false};
  // std::shared_ptr<CircularAudioArray> circularBuffer_;

  mutable std::mutex adapterNodeLock_;
  std::shared_ptr<RecorderAdapterNode> adapterNode_ = nullptr;

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
};

} // namespace audioapi
