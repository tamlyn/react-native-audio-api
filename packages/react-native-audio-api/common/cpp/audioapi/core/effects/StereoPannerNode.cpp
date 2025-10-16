#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/StereoPannerNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

// https://webaudio.github.io/web-audio-api/#stereopanner-algorithm

namespace audioapi {

StereoPannerNode::StereoPannerNode(BaseAudioContext *context)
    : AudioNode(context) {
  channelCountMode_ = ChannelCountMode::CLAMPED_MAX;
  panParam_ = std::make_shared<AudioParam>(0.0, -1.0f, 1.0f, context);
  isInitialized_ = true;
}

std::shared_ptr<AudioParam> StereoPannerNode::getPanParam() const {
  return panParam_;
}

std::shared_ptr<AudioBus> StereoPannerNode::processNode(
    const std::shared_ptr<AudioBus> &processingBus,
    int framesToProcess) {
  double time = context_->getCurrentTime();
  double deltaTime = 1.0 / context_->getSampleRate();

  // needs to be tested and probably fixed
  auto panValuesArray = panParam_->processARateParam(framesToProcess, time);
  const float *panParamValues = panValuesArray->getChannel(0)->getData();

  std::shared_ptr<AudioBus> outBus = nullptr;
  std::shared_ptr<AudioBus> tempBus = nullptr;
  bool usedTemp = false;

  try {
    outBus = getOutputBus(0);
  } catch (...) {
    outBus = nullptr;
  }

  if (!outBus || outBus->getNumberOfChannels() < 2) {
    tempBus = std::make_shared<AudioBus>(
        static_cast<size_t>(framesToProcess), 2, context_->getSampleRate());
    tempBus->zero();
    outBus = tempBus;
    usedTemp = true;
  } else {
    outBus->zero();
  }

  auto *outputLeft = outBus->getChannelByType(AudioBus::ChannelLeft);
  auto *outputRight = outBus->getChannelByType(AudioBus::ChannelRight);

  auto numInChannels = processingBus ? processingBus->getNumberOfChannels() : 0;
  auto *inputLeft = processingBus
      ? processingBus->getChannelByType(AudioBus::ChannelLeft)
      : nullptr;
  auto *inputRight = processingBus
      ? processingBus->getChannelByType(AudioBus::ChannelRight)
      : nullptr;

  if (!inputLeft) {
    if (usedTemp) {
      if (!m_outputBuses.empty() && m_outputBuses[0]) {
        m_outputBuses[0]->copy(outBus.get());
        return m_outputBuses[0];
      }
    }
    return outBus;
  }

  if (numInChannels <= 1 || inputRight == nullptr) {
    const float *inL = inputLeft->getData();
    float *outL = outputLeft->getData();
    float *outR = outputRight->getData();

    for (int i = 0; i < framesToProcess; ++i) {
      float pan = std::clamp(panParamValues[i], -1.0f, 1.0f);
      float x = (pan + 1.0f) * 0.5f;
      float gainL = static_cast<float>(cos(x * PI / 2.0));
      float gainR = static_cast<float>(sin(x * PI / 2.0));
      float inputSample = inL[i];

      outL[i] = inputSample * gainL;
      outR[i] = inputSample * gainR;

      time += deltaTime;
    }
  } else {
    const float *inL = inputLeft->getData();
    const float *inR = inputRight->getData();
    float *outL = outputLeft->getData();
    float *outR = outputRight->getData();

    for (int i = 0; i < framesToProcess; ++i) {
      float pan = std::clamp(panParamValues[i], -1.0f, 1.0f);
      float x = (pan <= 0.0f ? pan + 1.0f : pan);
      float gainL = static_cast<float>(cos(x * PI / 2.0));
      float gainR = static_cast<float>(sin(x * PI / 2.0));

      float sampleL = inL[i];
      float sampleR = inR[i];

      if (pan <= 0.0f) {
        outL[i] = sampleL + sampleR * gainL;
        outR[i] = sampleR * gainR;
      } else {
        outL[i] = sampleL * gainL;
        outR[i] = sampleR + sampleL * gainR;
      }

      time += deltaTime;
    }
  }

  if (usedTemp) {
    if (!m_outputBuses.empty() && m_outputBuses[0]) {
      m_outputBuses[0]->copy(outBus.get());
      return m_outputBuses[0];
    }
  }

  return outBus;
}

} // namespace audioapi
