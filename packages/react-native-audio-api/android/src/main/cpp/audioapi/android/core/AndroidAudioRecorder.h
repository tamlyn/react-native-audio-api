#pragma once

#include <audioapi/core/inputs/AudioRecorder.h>

#include <oboe/Oboe.h>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

#include <audioapi/android/core/NativeAudioRecorder.hpp>

namespace audioapi {

using namespace oboe;

class AudioBus;
class CircularAudioArray;
class AndroidFileWriterBackend;
class AudioEventHandlerRegistry;

class AndroidAudioRecorder : public AudioStreamDataCallback, public AudioRecorder {
 public:
  explicit AndroidAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  ~AndroidAudioRecorder() override;

  std::string start() override;
  std::tuple<std::string, double, double> stop() override;

  void enableFileOutput(
      float sampleRate,
      size_t channelCount,
      size_t bitRate,
      size_t iosFlags,
      size_t androidFlags) override;
  void disableFileOutput() override;

  void pause() override;
  void resume() override;

  void setOnAudioReadyCallback(float sampleRate, size_t bufferLength, size_t channelCount, uint64_t callbackId)
      override;
  void clearOnAudioReadyCallback() override;

  double getCurrentDuration() const override;


  DataCallbackResult onAudioReady(
          AudioStream *oboeStream,
          void *audioData,
          int32_t numFrames) override;

 private:
  std::string filePath_;
  std::shared_ptr<AndroidFileWriterBackend> fileWriter_;
  std::shared_ptr<AudioStream> mStream_;

  int32_t streamSampleRate_;
  int32_t streamChannelCount_;
  int32_t streamMaxBufferSizeInFrames_;

  facebook::jni::global_ref<NativeAudioRecorder> nativeAudioRecorder_;
};

} // namespace audioapi
