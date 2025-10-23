#pragma once

#include <audioapi/core/inputs/AudioRecorder.h>

#include <oboe/Oboe.h>
#include <functional>
#include <memory>
#include <string>

#include <audioapi/android/core/NativeAudioRecorder.hpp>

namespace audioapi {

using namespace oboe;

class AudioBus;
class CircularAudioArray;
class AndroidAudioFileWriter;
class AudioEventHandlerRegistry;

class AndroidAudioRecorder : public AudioStreamDataCallback, public AudioRecorder {
 public:
  explicit AndroidAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  ~AndroidAudioRecorder() override;

  void start() override;
  std::string stop() override;

  void enableFileOutput(
      float sampleRate,
      size_t channelCount,
      size_t bitRate,
      size_t iosFlags,
      size_t androidFlags) override;
  void disableFileOutput() override;

  void pause() override;
  void resume() override;

  DataCallbackResult onAudioReady(
          AudioStream *oboeStream,
          void *audioData,
          int32_t numFrames) override;

 private:
  std::shared_ptr<AndroidAudioFileWriter> fileWriter_;
  std::shared_ptr<AudioStream> mStream_;
  facebook::jni::global_ref<NativeAudioRecorder> nativeAudioRecorder_;
};

} // namespace audioapi
