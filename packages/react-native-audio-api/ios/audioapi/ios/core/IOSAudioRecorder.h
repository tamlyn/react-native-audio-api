#pragma once

#ifdef __OBJC__ // when compiled as C++
#import <NativeAudioRecorder.h>
#else
typedef struct objc_object AVAudioFile;
typedef struct objc_object NSURL;
typedef struct objc_object AudioBufferList;
typedef struct objc_object NativeAudioRecorder;
#endif // __OBJC__

#include <audioapi/core/inputs/AudioRecorder.h>

namespace audioapi {

class AudioBus;
class CircularAudioArray;
class IOSAudioFileWriter;
class AudioEventHandlerRegistry;

class IOSAudioRecorder : public AudioRecorder {
 public:
  IOSAudioRecorder(const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);
  ~IOSAudioRecorder() override;

  void start() override;
  std::string stop() override;

  void enableFileOutput(float sampleRate, size_t channelCount, size_t bitRate, size_t iosFlags, size_t androidFlags)
      override;
  void disableFileOutput() override;

  void pause() override;
  void resume() override;

 private:
  std::shared_ptr<IOSAudioFileWriter> fileWriter_;
  NativeAudioRecorder *nativeRecorder_;
};

} // namespace audioapi
