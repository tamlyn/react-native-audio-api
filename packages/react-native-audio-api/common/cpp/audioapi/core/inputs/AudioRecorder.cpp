#include <audioapi/HostObjects/sources/AudioBufferHostObject.h>
#include <audioapi/core/inputs/AudioRecorder.h>
#include <audioapi/core/sources/AudioBuffer.h>
#include <audioapi/core/sources/RecorderAdapterNode.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>

namespace audioapi {

AudioRecorder::AudioRecorder(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry)
    : audioEventHandlerRegistry_(audioEventHandlerRegistry) {
  isRunning_.store(false);
  fileOutputEnabled_.store(false);
  callbackOutputEnabled_.store(false);
  isConnected_.store(false);
}

bool AudioRecorder::isRecording() {
  return isRunning_.load();
}

void AudioRecorder::connect(const std::shared_ptr<RecorderAdapterNode> &node) {
  // node->init();
  // adapterNodeLock_.lock();
  // adapterNode_ = node;
  // adapterNodeLock_.unlock();
  // isConnected_.store(true);
  //   node->init(ringBufferSize_);
  //   adapterNodeLock_.lock();
  //   adapterNode_ = node;
  //   adapterNodeLock_.unlock();
}

void AudioRecorder::disconnect() {
  // adapterNodeLock_.lock();
  // adapterNode_ = nullptr;
  // adapterNodeLock_.unlock();
  // isConnected_.store(false);
}

void AudioRecorder::setOnAudioReadyCallback(
    float sampleRate,
    size_t bufferLength,
    size_t channelCount,
    uint64_t callbackId) {
  callbackProperties_.sampleRate = sampleRate;
  callbackProperties_.bufferLength = bufferLength;
  callbackProperties_.channelCount = channelCount;
  callbackProperties_.callbackId = callbackId;

  callbackOutputEnabled_.store(true);
}

void AudioRecorder::clearOnAudioReadyCallback() {
  callbackOutputEnabled_.store(false);
  callbackProperties_ = CallbackProperties{};
}

void AudioRecorder::invokeOnAudioReadyCallback(
    const std::shared_ptr<AudioBus> &bus,
    int numFrames) {
  //   if (!hasCallback()) {
  //     return;
  //   }

  //   auto audioBuffer = std::make_shared<AudioBuffer>(bus);
  //   auto audioBufferHostObject =
  //       std::make_shared<AudioBufferHostObject>(audioBuffer);

  //   std::unordered_map<std::string, EventValue> body = {};
  //   body.insert({"buffer", audioBufferHostObject});
  //   body.insert({"numFrames", numFrames});

  //   if (audioEventHandlerRegistry_ != nullptr) {
  //     audioEventHandlerRegistry_->invokeHandlerWithEventBody(
  //         "audioReady", onAudioReadyCallbackId_, body);
  //   }
}

void AudioRecorder::sendRemainingCallbackData() {
  //   if (!hasCallback()) {
  //     return;
  //   }

  //   auto bus = std::make_shared<AudioBus>(
  //       circularBuffer_->getNumberOfAvailableFrames(), 1, sampleRate_);
  //   auto *outputChannel = bus->getChannel(0)->getData();
  //   auto availableFrames =
  //       static_cast<int>(circularBuffer_->getNumberOfAvailableFrames());

  //   circularBuffer_->pop_front(
  //       outputChannel, circularBuffer_->getNumberOfAvailableFrames());

  //   invokeOnAudioReadyCallback(bus, availableFrames);
}

bool AudioRecorder::usesCallback() const {
  return callbackOutputEnabled_.load();
}

bool AudioRecorder::usesFileOutput() const {
  return fileOutputEnabled_.load();
}

bool AudioRecorder::isConnected() const {
  return isConnected_.load();
}

// AudioRecorder::AudioRecorder(
//     const std::shared_ptr<AudioEventHandlerRegistry>
//     &audioEventHandlerRegistry)
//       audioEventHandlerRegistry_(audioEventHandlerRegistry) {
//   constexpr int minRingBufferSize = 8192;
//   ringBufferSize_ = std::max(2 * bufferLength, minRingBufferSize);

//   circularBuffer_ = std::make_shared<CircularAudioArray>(ringBufferSize_);
//   isRunning_.store(false);
// }

// void AudioRecorder::writeToBuffers(const float *data, int numFrames) {
//   if (adapterNodeLock_.try_lock()) {
//     if (adapterNode_ != nullptr) {
//       adapterNode_->buff_->write(data, numFrames);
//     }
//     adapterNodeLock_.unlock();
//   }

//   if (hasCallback()) {
//     circularBuffer_->push_back(data, numFrames);
//   }
// }

} // namespace audioapi
