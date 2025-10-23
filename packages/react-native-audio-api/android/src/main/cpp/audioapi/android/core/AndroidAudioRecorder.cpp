#include <audioapi/android/core/AndroidAudioRecorder.h>
#include <audioapi/android/core/utils/AndroidAudioFileWriter.h>
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
    printf("Audio stream is not initialized.\n");
    return;
  }

  if (usesFileOutput()) {
    fileWriter_->openFile();
  }

  if (usesCallback()) {
    // TODO: create circular buffer and converter?
  }

  if (isConnected()) {
    // TODO: set adapter node properties?
  }

  // if (mStream_) {
  //   nativeAudioRecorder_->start();
  //   mStream_->requestStart();
  // }

  nativeAudioRecorder_->start();
  mStream_->requestStart();
  isRunning_.store(true);
}

std::string AndroidAudioRecorder::stop() {
  if (!isRecording()) {
    return "";
  }

  if (!mStream_ || !nativeAudioRecorder_) {
    printf("Audio stream is not initialized.\n");
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
  fileOutputEnabled_.store(true);
  fileWriter_ = std::make_shared<AndroidAudioFileWriter>(
      sampleRate, channelCount, bitRate, androidFlags);
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

  return DataCallbackResult::Continue;
}

} // namespace audioapi
