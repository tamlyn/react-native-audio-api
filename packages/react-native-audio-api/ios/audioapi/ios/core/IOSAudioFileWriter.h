#pragma once

#ifndef __OBJC__ // when compiled as C++
typedef struct objc_object AVAudioFile;
typedef struct objc_object NSURL;
typedef struct objc_object AudioBufferList;
#endif // __OBJC__

namespace audioapi {

class IOSAudioFileWriter {
 public:
  IOSAudioFileWriter();
  ~IOSAudioFileWriter();

  void openFile();
  std::string closeFile();

  bool writeAudioData(const AudioBufferList *audioBufferList, int numFrames);

 private:
  NSDictionary *fileSettings_;
  AVAudioFile *audioFile_;
  NSURL *fileURL_;
};

} // namespace audioapi
