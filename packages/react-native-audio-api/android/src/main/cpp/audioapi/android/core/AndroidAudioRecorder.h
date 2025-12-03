#pragma once

#include <audioapi/core/inputs/AudioRecorder.h>
#include <oboe/Oboe.h>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <mutex>
#include <audioapi/utils/ReturnStatus.hpp>
#include <audioapi/android/core/NativeAudioRecorder.hpp>

namespace audioapi {

using namespace oboe;

class AudioBus;
class CircularAudioArray;
class AudioFileProperties;
class AndroidRecorderCallback;
class AndroidFileWriterBackend;
class AudioEventHandlerRegistry;

class AndroidAudioRecorder : public AudioStreamCallback, public AudioRecorder {
 public:
  explicit AndroidAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  ~AndroidAudioRecorder() override;
  void cleanup();

  ReturnStatus<std::string> start() override;
  ReturnStatus<std::tuple<std::string, double, double>> stop() override;

  ReturnStatus<std::string> enableFileOutput(std::shared_ptr<AudioFileProperties> properties) override;
  void disableFileOutput() override;

  void pause() override;
  void resume() override;
  bool isRecording() const override;
  bool isPaused() const override;
  bool isIdle() const override;

  ReturnStatus<void> setOnAudioReadyCallback(float sampleRate, size_t bufferLength, int channelCount, uint64_t callbackId)
      override;
  void clearOnAudioReadyCallback() override;

  void setOnErrorCallback(uint64_t callbackId) override;
  void clearOnErrorCallback() override;

  double getCurrentDuration() const override;

  DataCallbackResult onAudioReady(
          AudioStream *oboeStream,
          void *audioData,
          int32_t numFrames) override;
  void onErrorAfterClose(AudioStream *oboeStream, Result error) override;

 private:
  std::string filePath_;
  std::shared_ptr<AudioStream> mStream_;

  std::mutex callbackMutex_;
  std::mutex fileWriterMutex_;

  std::shared_ptr<AndroidFileWriterBackend> fileWriter_;
  std::shared_ptr<AndroidRecorderCallback> callback_;

  float streamSampleRate_;
  int32_t streamChannelCount_;
  int32_t streamMaxBufferSizeInFrames_;

  facebook::jni::global_ref<NativeAudioRecorder> nativeAudioRecorder_;

  ReturnStatus<void> openAudioStream();
};

} // namespace audioapi
