#pragma once

#include <audioapi/core/AudioNode.h>
#include <audioapi/core/types/OverSampleType.h>

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
    std::shared_ptr<AudioBus>
    processNode(const std::shared_ptr<AudioBus> &processingBus, int framesToProcess) override;

private:
  OverSampleType oversample_;
  std::shared_ptr<AudioArray> curve_ {};

    static OverSampleType fromString(const std::string &type) {
        std::string lowerType = type;
        std::transform(
                lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);

        if (lowerType == "2x")
            return OverSampleType::TWICE;
        if (lowerType == "4x")
            return OverSampleType::FOUR_TIMES;

        return OverSampleType::NONE;
    }

    static std::string toString(OverSampleType type) {
        switch (type) {
            case OverSampleType::TWICE:
                return "2x";
            case OverSampleType::FOUR_TIMES:
                return "4x";
            default:
                return "none";
        }
    }
};

} // namespace audioapi
