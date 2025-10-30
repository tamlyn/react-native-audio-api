#include <android/log.h>
#include <audioapi/HostObjects/sources/AudioBufferHostObject.h>
#include <audioapi/android/core/utils/AndroidRecorderCallback.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/libs/miniaudio/miniaudio.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>

#include <memory>

namespace audioapi {

AndroidRecorderCallback::AndroidRecorderCallback(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry,
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId)
    : sampleRate_(sampleRate),
      bufferLength_(bufferLength),
      channelCount_(channelCount),
      callbackId_(callbackId),
      audioEventHandlerRegistry_(audioEventHandlerRegistry) {
  ringBufferSize_ = std::max((int)bufferLength * 2, 8192);
  circularBus_.resize(channelCount_);

  for (size_t i = 0; i < channelCount_; ++i) {
    auto busArray = std::make_shared<CircularAudioArray>(ringBufferSize_);
    circularBus_[i] = busArray;
  }
}

AndroidRecorderCallback::~AndroidRecorderCallback() {
  for (size_t i = 0; i < circularBus_.size(); ++i) {
    circularBus_[i].reset();
  }
}

void AndroidRecorderCallback::prepare(
    int32_t streamSampleRate,
    int32_t streamChannelCount,
    size_t maxInputBufferLength) {
  ma_result result;

  streamSampleRate_ = streamSampleRate;
  streamChannelCount_ = streamChannelCount;
  maxInputBufferLength_ = maxInputBufferLength;

  ma_data_converter_config converterConfig = ma_data_converter_config_init(
      ma_format_f32,
      ma_format_f32,
      streamChannelCount_,
      channelCount_,
      streamSampleRate_,
      static_cast<int32_t>(sampleRate_));

  converter_ = std::make_unique<ma_data_converter>();
  result = ma_data_converter_init(&converterConfig, NULL, converter_.get());

  if (result != MA_SUCCESS) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        "AndroidRecorderCallback",
        "Failed to initialize miniaudio data converter");
    return;
  }

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), maxInputBufferLength_, &processingBufferLength_);

  processingBufferLength_ =
      std::max(processingBufferLength_, (ma_uint64)maxInputBufferLength_);

  deinterleavingArray_ = std::make_shared<AudioArray>(processingBufferLength_);
  processingBuffer_ = ma_malloc(
      processingBufferLength_ * channelCount_ *
          ma_get_bytes_per_sample(ma_format_f32),
      NULL);
}

void AndroidRecorderCallback::cleanup() {
  if (converter_ != nullptr) {
    ma_data_converter_uninit(converter_.get(), NULL);
    converter_.reset();
  }

  if (processingBuffer_ != nullptr) {
    ma_free(processingBuffer_, NULL);
    processingBuffer_ = nullptr;
    processingBufferLength_ = 0;
  }

  for (size_t i = 0; i < circularBus_.size(); ++i) {
    circularBus_[i].reset();
  }
}

void AndroidRecorderCallback::receiveAudioData(void *data, int numFrames) {
  ma_uint64 inputFrameCount = numFrames;
  ma_uint64 outputFrameCount = 0;

  if ((float)streamSampleRate_ == sampleRate_ ||
      streamChannelCount_ == channelCount_) {
    deinterleaveAndWriteAudioData(data, numFrames);
    emitAudioData();
    return;
  }

  ma_data_converter_get_expected_output_frame_count(
      converter_.get(), inputFrameCount, &outputFrameCount);

  ma_data_converter_process_pcm_frames(
      converter_.get(),
      data,
      &inputFrameCount,
      processingBuffer_,
      &outputFrameCount);

  deinterleaveAndWriteAudioData(
      processingBuffer_, static_cast<int>(outputFrameCount));
  emitAudioData();
}

void AndroidRecorderCallback::deinterleaveAndWriteAudioData(
    void *data,
    int numFrames) {
  auto *inputData = static_cast<float *>(data);

  for (size_t channel = 0; channel < channelCount_; ++channel) {
    float *channelData = deinterleavingArray_->getData();

    for (int frame = 0; frame < numFrames; ++frame) {
      channelData[frame] = inputData[frame * streamChannelCount_ + channel];
    }

    circularBus_[channel]->push_back(channelData, numFrames);
  }
}

void AndroidRecorderCallback::emitAudioData() {
  while (circularBus_[0]->getNumberOfAvailableFrames() >= bufferLength_) {
    auto bus =
        std::make_shared<AudioBus>(bufferLength_, channelCount_, sampleRate_);

    for (int i = 0; i < channelCount_; ++i) {
      auto *outputChannel = bus->getChannel(i)->getData();
      circularBus_[i]->pop_front(outputChannel, bufferLength_);
    }

    invokeCallback(bus, (int)bufferLength_);
  }
}

void AndroidRecorderCallback::invokeCallback(
    const std::shared_ptr<AudioBus> &bus,
    int numFrames) {
  auto audioBuffer = std::make_shared<AudioBuffer>(bus);
  auto audioBufferHostObject =
      std::make_shared<AudioBufferHostObject>(audioBuffer);

  std::unordered_map<std::string, EventValue> eventPayload = {};
  eventPayload.insert({"buffer", audioBufferHostObject});
  eventPayload.insert({"numFrames", numFrames});

  if (audioEventHandlerRegistry_) {
    audioEventHandlerRegistry_->invokeHandlerWithEventBody(
        "audioReady", callbackId_, eventPayload);
  }
}

void AndroidRecorderCallback::sendRemainingData() {
  auto numberOfFrames = circularBus_[0]->getNumberOfAvailableFrames();
  auto bus = std::make_shared<AudioBus>(
      circularBus_[0]->getNumberOfAvailableFrames(),
      channelCount_,
      sampleRate_);

  for (int i = 0; i < channelCount_; ++i) {
    auto *outputChannel = bus->getChannel(i)->getData();
    circularBus_[i]->pop_front(outputChannel, numberOfFrames);
  }

  invokeCallback(bus, (int)numberOfFrames);
}

} // namespace audioapi
