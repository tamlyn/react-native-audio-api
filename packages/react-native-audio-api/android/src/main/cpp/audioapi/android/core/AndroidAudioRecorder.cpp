#include <android/log.h>
#include <audioapi/android/core/AndroidAudioRecorder.h>
#include <audioapi/android/core/utils/AndroidFileWriterBackend.h>
#include <audioapi/android/core/utils/miniaudioBackend/MiniAudioFileWriter.h>
#include <audioapi/core/sources/RecorderAdapterNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/CircularOverflowableAudioArray.h>

namespace audioapi {

AndroidAudioRecorder::AndroidAudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : AudioRecorder(audioEventHandlerRegistry) {
  AudioStreamBuilder builder;

  builder.setSharingMode(SharingMode::Exclusive)
      ->setDirection(Direction::Input)
      ->setFormat(AudioFormat::Float)
      ->setFormatConversionAllowed(true)
      ->setPerformanceMode(PerformanceMode::None)
      ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
      ->setDataCallback(this)
      ->openStream(mStream_);

  streamSampleRate_ = mStream_->getSampleRate();
  streamChannelCount_ = mStream_->getChannelCount();
  streamMaxBufferSizeInFrames_ = mStream_->getBufferSizeInFrames();

  nativeAudioRecorder_ = jni::make_global(NativeAudioRecorder::create());
}

AndroidAudioRecorder::~AndroidAudioRecorder() {
  nativeAudioRecorder_.release();

  if (mStream_) {
    mStream_->requestStop();
    mStream_->close();
    mStream_.reset();
  }
}

void AndroidAudioRecorder::start() {
  if (isRecording()) {
    return;
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return;
  }

  if (usesFileOutput()) {
    fileWriter_->openFile(
        streamSampleRate_, streamChannelCount_, streamMaxBufferSizeInFrames_);
  }

  if (usesCallback()) {
    // TODO: create circular buffer and converter?
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  nativeAudioRecorder_->start();
  mStream_->requestStart();
  isRunning_.store(true);
}

std::string AndroidAudioRecorder::stop() {
  if (!isRecording()) {
    return "";
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidAudioRecorder",
        "Audio stream is not initialized.\n");
    return "";
  }

  nativeAudioRecorder_->stop();
  mStream_->requestStop();
  isRunning_.store(false);

  // TODO: sendRemainingData() ?

  if (usesFileOutput()) {
    return fileWriter_->closeFile();
  }

  return "";
}

void AndroidAudioRecorder::enableFileOutput(
    float sampleRate,
    size_t channelCount,
    size_t bitRate,
    size_t iosFlags,
    size_t androidFlags) {
  uint8_t format = static_cast<uint8_t>(androidFlags & 0xF);

  if (format == 1) {
    fileWriter_ = std::make_shared<MiniAudioFileWriter>(
        sampleRate, channelCount, bitRate, androidFlags);
    fileOutputEnabled_.store(true);
    return;
  }
}

void AndroidAudioRecorder::disableFileOutput() {
  fileOutputEnabled_.store(false);
  fileWriter_ = nullptr;
}

void AndroidAudioRecorder::pause() {}

void AndroidAudioRecorder::resume() {}

DataCallbackResult AndroidAudioRecorder::onAudioReady(
    oboe::AudioStream *oboeStream,
    void *audioData,
    int32_t numFrames) {
  if (usesFileOutput()) {
    fileWriter_->writeAudioData(audioData, numFrames);
  }

  return DataCallbackResult::Continue;
}

} // namespace audioapi

// if (isRunning_.load()) {
//   auto *inputChannel = static_cast<float *>(audioData);
//   writeToBuffers(inputChannel, numFrames);
// }

// while (circularBuffer_->getNumberOfAvailableFrames() >= bufferLength_) {
//   auto bus = std::make_shared<AudioBus>(bufferLength_, 1, sampleRate_);
//   auto *outputChannel = bus->getChannel(0)->getData();

//   circularBuffer_->pop_front(outputChannel, bufferLength_);

//   invokeOnAudioReadyCallback(bus, bufferLength_);
// }
