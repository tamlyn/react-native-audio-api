
#include <android/log.h>
#include <audioapi/android/core/utils/AndroidAudioFileOptions.h>
#include <audioapi/android/core/utils/AndroidAudioFileWriter.h>
#include <audioapi/android/core/utils/FileBackend.h>
#include <audioapi/android/core/utils/MiniaudioFileBackend.h>

namespace audioapi {

AndroidAudioFileWriter::AndroidAudioFileWriter(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t androidFlags) {
  fileOptions_ = std::make_shared<AndroidAudioFileOptions>(
      sampleRate, channelCount, bitRate, androidFlags);

  AudioFormat format = fileOptions_->getFormat();

  if (format == AudioFormat::WAV) {
    fileBackend_ = std::make_shared<MiniaudioFileBackend>(fileOptions_);
  }
}

AndroidAudioFileWriter::~AndroidAudioFileWriter() {
  fileBackend_.reset();
}

void AndroidAudioFileWriter::openFile(
    int32_t streamSampleRate,
    int32_t streamChannelCount) {
  std::string fileName = fileOptions_->getFilePath("audio");
  __android_log_print(
      ANDROID_LOG_INFO,
      "AndroidAudioFileWriter",
      "Opening audio file at path: %s",
      fileName.c_str());

  fileBackend_->openFile(fileName, streamSampleRate, streamChannelCount);
}

std::string AndroidAudioFileWriter::closeFile() {
  return fileBackend_->closeFile();
}

bool AndroidAudioFileWriter::writeAudioData(void *inputBus, int numFrames) {
  return fileBackend_->writeAudioData(inputBus, numFrames);
}

} // namespace audioapi
