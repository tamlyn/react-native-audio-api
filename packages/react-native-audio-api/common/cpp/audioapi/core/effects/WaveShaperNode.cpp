#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/WaveShaperNode.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

WaveShaperNode::WaveShaperNode(BaseAudioContext *context)
    : AudioNode(context), oversample_(OverSampleType::OVERSAMPLE_NONE) {
  isInitialized_ = true;
}

std::string WaveShaperNode::getOversample() const {
  return toString(oversample_);
}

void WaveShaperNode::setOversample(const std::string &type) {
  curveMutex_.lock();
  oversample_ = fromString(type);
  curveMutex_.unlock();
}

std::shared_ptr<AudioArray> WaveShaperNode::getCurve() const {
  return curve_;
}

void WaveShaperNode::setCurve(const std::shared_ptr<AudioArray> &curve) {
  curveMutex_.lock();
  curve_ = curve;
  curveMutex_.unlock();
}

std::shared_ptr<AudioBus> WaveShaperNode::processNode(
    const std::shared_ptr<AudioBus> &processingBus,
    int framesToProcess) {
  if (!isInitialized_) {
    return processingBus;
  }

  if (!curveMutex_.try_lock()) {
    return processingBus;
  }
  if (!curve_) {
      return processingBus;
  }

  auto curveArray = curve_->getData();

  for (int channel = 0; channel < processingBus->getNumberOfChannels();
       channel += 1) {
      auto channelData = processingBus->getChannel(channel)->getData();
      for (int i = 0; i < framesToProcess; i += 1) {
          float v = (curve_->getSize() - 1) * 0.5f * (channelData[i] + 1.0f);

          if (v < 0)
              channelData[i] = curveArray[0];
          else if (v >= curve_->getSize() - 1)
              channelData[i] = curveArray[curve_->getSize() - 1];
          else {
              auto k = std::floor(v);
              auto f = v - k;
              unsigned kIndex = k;
              channelData[i] = (1 - f) * curveArray[kIndex] + f * curveArray[kIndex + 1];
          }
      }
  }

  curveMutex_.unlock();

  return processingBus;
}

} // namespace audioapi
