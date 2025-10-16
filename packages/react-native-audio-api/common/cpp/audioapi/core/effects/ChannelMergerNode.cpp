#include "ChannelMergerNode.h"
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

ChannelMergerNode::ChannelMergerNode(
    BaseAudioContext *context,
    unsigned numberOfInputs)
    : AudioNode(context) {
  numberOfInputs_ = numberOfInputs;
  numberOfOutputs_ = 1;

  channelCount_ = 1;
  channelCountMode_ = ChannelCountMode::EXPLICIT;
  channelInterpretation_ = ChannelInterpretation::SPEAKERS;

  m_outputBuses.clear();
  m_outputBuses.push_back(
      std::make_shared<AudioBus>(
          RENDER_QUANTUM_SIZE, numberOfInputs_, context->getSampleRate()));

  isInitialized_ = true;
}

void ChannelMergerNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  auto outputBus = getOutputBus(0);

  for (unsigned i = 0; i < numberOfInputs_; ++i) {
    bool isInputConnected = i < inputBuses.size() && inputBuses[i];

    if (isInputConnected) {
      const auto &sourceBus = inputBuses[i];
      outputBus->getChannel(i)->copy(sourceBus->getChannel(0));
    } else {
      outputBus->getChannel(i)->zero();
    }
  }
}

} // namespace audioapi