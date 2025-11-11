#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/WaveShaperNode.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

WaveShaperNode::WaveShaperNode(BaseAudioContext *context)
    : AudioNode(context), oversample_(OverSampleType::NONE) {
  isInitialized_ = true;
}

std::string WaveShaperNode::getOversample() const {
  return toString(oversample_);
}

void WaveShaperNode::setOversample(const std::string &type) {
  oversample_ = fromString(type);
}

std::shared_ptr<AudioArray> WaveShaperNode::getCurve() const {
  return curve_;
}

void WaveShaperNode::setCurve(const std::shared_ptr<AudioArray> &curve) {
  curve_ = curve;
}

std::shared_ptr<AudioBus>
WaveShaperNode::processNode(const std::shared_ptr<AudioBus> &processingBus,
                            int framesToProcess) {
    if (!curve_) {
      return processingBus;
    }

    return audioBus_;
}

} // namespace audioapi
