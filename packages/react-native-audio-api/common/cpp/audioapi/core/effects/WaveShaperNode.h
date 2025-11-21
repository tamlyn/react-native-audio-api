#pragma once

#include <audioapi/core/AudioNode.h>
#include <audioapi/core/types/OverSampleType.h>
#include <audioapi/dsp/Resampler.h>

#include <algorithm>
#include <memory>
#include <string>

namespace audioapi {

class AudioBus;
class AudioArray;

class WaveShaperNode : public AudioNode {
 public:
  explicit WaveShaperNode(BaseAudioContext *context);

  [[nodiscard]] std::string getOversample() const;
  [[nodiscard]] std::shared_ptr<AudioArray> getCurve() const;

  void setOversample(const std::string &type);
  void setCurve(const std::shared_ptr<AudioArray> &curve);

 protected:
  std::shared_ptr<AudioBus> processNode(
      const std::shared_ptr<AudioBus> &processingBus,
      int framesToProcess) override;

 private:
  std::atomic<OverSampleType> oversample_;
  std::shared_ptr<AudioArray> curve_{};
  mutable std::mutex curveMutex_;

  // stage 1 Filters (1x <-> 2x)
  std::unique_ptr<UpSampler> upSampler_;
  std::unique_ptr<DownSampler> downSampler_;

  // stage 2 Filters (2x <-> 4x)
  std::unique_ptr<UpSampler> upSampler2_;
  std::unique_ptr<DownSampler> downSampler2_;

  std::shared_ptr<AudioArray> tempBuffer2x_;
  std::shared_ptr<AudioArray> tempBuffer4x_;

  void process(const std::shared_ptr<AudioArray> &channelData);

  void process2x(const std::shared_ptr<AudioArray> &channelData);

  void process4x(const std::shared_ptr<AudioArray> &channelData);

  static OverSampleType fromString(const std::string &type) {
    std::string lowerType = type;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);

    if (lowerType == "2x")
      return OverSampleType::OVERSAMPLE_2X;
    if (lowerType == "4x")
      return OverSampleType::OVERSAMPLE_4X;

    return OverSampleType::OVERSAMPLE_NONE;
  }

  static std::string toString(OverSampleType type) {
    switch (type) {
      case OverSampleType::OVERSAMPLE_2X:
        return "2x";
      case OverSampleType::OVERSAMPLE_4X:
        return "4x";
      default:
        return "none";
    }
  }
};

} // namespace audioapi
