#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/WaveShaperNode.h>
#include <audioapi/dsp/Resampler.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

WaveShaperNode::WaveShaperNode(BaseAudioContext *context)
    : AudioNode(context), oversample_(OverSampleType::OVERSAMPLE_NONE) {
  resampler_ = std::make_shared<Resampler>();
  tempArray2x_ = std::make_shared<AudioArray>(2 * RENDER_QUANTUM_SIZE);
  tempArray4x_ = std::make_shared<AudioArray>(4 * RENDER_QUANTUM_SIZE);
  isInitialized_ = true;
}

std::string WaveShaperNode::getOversample() const {
  return toString(oversample_.load(std::memory_order_acquire));
}

void WaveShaperNode::setOversample(const std::string &type) {
  oversample_.store(fromString(type), std::memory_order_release);
}

std::shared_ptr<AudioArray> WaveShaperNode::getCurve() const {
  std::lock_guard<std::mutex> lock(curveMutex_);
  return curve_;
}

void WaveShaperNode::setCurve(const std::shared_ptr<AudioArray> &curve) {
  std::lock_guard<std::mutex> lock(curveMutex_);
  curve_ = curve;
}

std::shared_ptr<AudioBus> WaveShaperNode::processNode(
    const std::shared_ptr<AudioBus> &processingBus,
    int framesToProcess) {
  if (!isInitialized_) {
    return processingBus;
  }

  std::unique_lock<std::mutex> lock(curveMutex_, std::try_to_lock);

  if (!lock.owns_lock()) {
    return processingBus;
  }

  if (!curve_) {
    return processingBus;
  }

  auto oversample = oversample_.load(std::memory_order_acquire);

  for (int channel = 0; channel < processingBus->getNumberOfChannels();
       channel += 1) {
    auto channelData = processingBus->getSharedChannel(channel);

    switch (oversample) {
      case OverSampleType::OVERSAMPLE_2X:
        process2x(channelData);
      case OverSampleType::OVERSAMPLE_4X:
        process4x(channelData);
      case OverSampleType::OVERSAMPLE_NONE:
      default:
        process(channelData);
    }
  }

  return processingBus;
}

void WaveShaperNode::process(const std::shared_ptr<AudioArray> &channelData) {
  auto curveArray = curve_->getData();
  auto curveSize = curve_->getSize();

  auto data = channelData->getData();

  for (int i = 0; i < channelData->getSize(); i += 1) {
    float v = (static_cast<float>(curveSize) - 1) * 0.5f * (data[i] + 1.0f);

    if (v < 0)
      data[i] = curveArray[0];
    else if (v >= static_cast<float>(curveSize) - 1)
      data[i] = curveArray[curveSize - 1];
    else {
      auto k = std::floor(v);
      auto f = v - k;
      auto kIndex = static_cast<size_t>(
          std::clamp(k, 0.0f, static_cast<float>(curveSize) - 1));
      data[i] = (1 - f) * curveArray[kIndex] + f * curveArray[kIndex + 1];
    }
  }
}

void WaveShaperNode::process2x(const std::shared_ptr<AudioArray> &channelData) {
  resampler_->process(channelData, tempArray2x_);
  process(tempArray2x_);
  resampler_->process(tempArray2x_, channelData);
}

void WaveShaperNode::process4x(const std::shared_ptr<AudioArray> &channelData) {
  resampler_->process(channelData, tempArray4x_);
  process(tempArray4x_);
  resampler_->process(tempArray4x_, channelData);
}

} // namespace audioapi
