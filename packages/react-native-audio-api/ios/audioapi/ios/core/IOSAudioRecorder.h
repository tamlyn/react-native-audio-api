#pragma once

#ifdef __OBJC__ // when compiled as Objective-C
#import <NativeAudioRecorder.h>
#else
typedef struct objc_object NSURL;
typedef struct objc_object AVAudioFile;
typedef struct objc_object AudioBufferList;
typedef struct objc_object NativeAudioRecorder;
#endif // __OBJC__

#include <audioapi/core/inputs/AudioRecorder.h>
#include <audioapi/utils/ReturnStatus.hpp>

#include <mutex>

namespace audioapi {

class FileWriter;
class RecorderCallback;
class AudioFileProperties;
class AudioEventHandlerRegistry;

class IOSAudioRecorder : public AudioRecorder {
 public:
  IOSAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  ~IOSAudioRecorder() override;

  ReturnStatus<std::string> start() override;
  ReturnStatus<std::tuple<std::string, double, double>> stop() override;

  ReturnStatus<std::string> enableFileOutput(
      std::shared_ptr<AudioFileProperties> properties) override;
  void disableFileOutput() override;

  void pause() override;
  void resume() override;

  bool isRecording() const override;
  bool isPaused() const override;
  bool isIdle() const override;

  ReturnStatus<void> setOnAudioReadyCallback(
      float sampleRate,
      size_t bufferLength,
      size_t channelCount,
      uint64_t callbackId) override;
  void clearOnAudioReadyCallback() override;

  void setOnErrorCallback(uint64_t callbackId) override;
  void clearOnErrorCallback() override;

  double getCurrentDuration() const override;

 private:
  std::shared_ptr<FileWriter> fileWriter_;
  std::shared_ptr<RecorderCallback> callback_;
  std::string filePath_{""};

  std::mutex callbackMutex_;
  std::mutex fileWriterMutex_;

  std::atomic<uint64_t> errorCallbackId_{0};

  NativeAudioRecorder *nativeRecorder_;
};

} // namespace audioapi
