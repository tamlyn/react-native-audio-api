#pragma once

#include <audioapi/core/utils/Locker.h>
#include <audioapi/utils/ReturnStatus.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>

namespace audioapi {

class AudioBus;
class CircularAudioArray;
class RecorderAdapterNode;
class AudioFileProperties;
class AudioEventHandlerRegistry;

class AudioRecorder {
 public:
  enum class RecorderState { Idle = 0, Recording, Paused };
  explicit AudioRecorder(
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
      : audioEventHandlerRegistry_(audioEventHandlerRegistry) {}
  virtual ~AudioRecorder() = default;

  virtual ReturnStatus<std::string> start() = 0;
  virtual ReturnStatus<std::tuple<std::string, double, double>> stop() = 0;

  virtual ReturnStatus<std::string> enableFileOutput(
      std::shared_ptr<AudioFileProperties> properties) = 0;
  virtual void disableFileOutput() = 0;

  virtual void pause() = 0;
  virtual void resume() = 0;

  virtual void connect(const std::shared_ptr<RecorderAdapterNode> &node) = 0;
  virtual void disconnect() = 0;

  virtual ReturnStatus<void> setOnAudioReadyCallback(
      float sampleRate,
      size_t bufferLength,
      int channelCount,
      uint64_t callbackId) = 0;
  virtual void clearOnAudioReadyCallback() = 0;

  virtual void setOnErrorCallback(uint64_t callbackId) = 0;
  virtual void clearOnErrorCallback() = 0;

  virtual double getCurrentDuration() const = 0;

  bool usesCallback() const {
    return callbackOutputEnabled_.load(std::memory_order_acquire);
  }

  bool usesFileOutput() const {
    return fileOutputEnabled_.load(std::memory_order_acquire);
  }

  bool isConnected() const {
    return isConnected_.load(std::memory_order_acquire);
  }

  virtual bool isRecording() const = 0;
  virtual bool isPaused() const = 0;
  virtual bool isIdle() const = 0;

 protected:
  std::atomic<RecorderState> state_{RecorderState::Idle};
  std::atomic<bool> fileOutputEnabled_{false};
  std::atomic<bool> callbackOutputEnabled_{false};
  std::atomic<bool> isConnected_{false};

  mutable std::mutex adapterNodeMutex_;
  std::shared_ptr<RecorderAdapterNode> adapterNode_ = nullptr;

  std::shared_ptr<AudioEventHandlerRegistry> audioEventHandlerRegistry_;
};

} // namespace audioapi
