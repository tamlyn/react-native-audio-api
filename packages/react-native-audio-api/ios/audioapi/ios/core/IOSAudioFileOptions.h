#pragma once

#ifndef __OBJC__ // when compiled as C++
// typedef struct objc_object AVAudioFile;
// typedef struct objc_object NSURL;
// typedef struct objc_object AudioBufferList;
#endif // __OBJC__

namespace audioapi {

class IOSAudioFileOptions {
 public:
  IOSAudioFileOptions(float sampleRate, size_t channelCount, size_t bitRate, size_t flags);
  ~IOSAudioFileOptions() = default;

  AudioFormatID getFormat() const;
  NSInteger getQuality() const;
  NSInteger getFlacCompressionLevel() const;
  NSSearchPathDirectory getDirectory() const;
  NSString *getFileExtension() const;
  NSInteger getBitDepth() const;
  NSDictionary *getFileSettings();

 private:
  uint8_t format_;
  uint8_t quality_;
  uint8_t flacCompressionLevel_;
  uint8_t directory_;
  uint8_t bitDepth_;
  float sampleRate_;
  size_t channelCount_;
  size_t bitRate_;
};

} // namespace audioapi
