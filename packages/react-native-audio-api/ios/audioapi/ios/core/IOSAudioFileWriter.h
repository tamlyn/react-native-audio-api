#pragma once

#include <memory>
#include <string>

#ifndef __OBJC__ // when compiled as C++
typedef struct objc_object AVAudioFile;
typedef struct objc_object NSURL;
typedef struct objc_object AudioBufferList;
#endif // __OBJC__

namespace audioapi {

class IOSAudioFileOptions;

class IOSAudioFileWriter {
 public:
  IOSAudioFileWriter(float sampleRate, size_t channelCount, size_t bitRate, size_t iosFlags);
  ~IOSAudioFileWriter();

  void openFile(AVAudioFormat *bufferFormat);
  std::string closeFile();

  bool writeAudioData(const AudioBufferList *audioBufferList, int numFrames);

 private:
  NSString *getISODateStringForDirectory();
  NSString *getTimestampForFilename();
  NSURL *getFileURL();

  std::shared_ptr<IOSAudioFileOptions> fileOptions_;
  AVAudioFile *audioFile_;
  AVAudioFormat *bufferFormat_;
  AVAudioConverter *converter_;
  NSURL *fileURL_;
};

} // namespace audioapi
