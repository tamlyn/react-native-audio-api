#pragma once

#ifdef __OBJC__ // when compiled as Objective-C++
#import <NativeAudioRecorder.h>
#else // when compiled as C++
typedef struct objc_object NativeAudioRecorder;
typedef struct objc_object AVAudioFile;
typedef struct objc_object NSURL;
typedef struct objc_object AudioBufferList;
#endif // __OBJC__

#include <audioapi/core/inputs/AudioRecorder.h>

namespace audioapi {

class AudioBus;
class CircularAudioArray;

class IOSAudioRecorder : public AudioRecorder {
 public:
  IOSAudioRecorder(
      float sampleRate,
      int bufferLength,
      bool recordToFile,
      const std::string &fileDirectory,
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);

  ~IOSAudioRecorder() override;

  void start() override;
  void stop() override;

  void writeToFile(const AudioBufferList *inputBuffer, int numFrames);

 private:
  NativeAudioRecorder *audioRecorder_;
  AVAudioFile *currentAudioFile_;

  void createFileForWriting();
  void releaseFile();

  NSURL *getFileURL();
  NSURL *currentFileURL_;
};

} // namespace audioapi
