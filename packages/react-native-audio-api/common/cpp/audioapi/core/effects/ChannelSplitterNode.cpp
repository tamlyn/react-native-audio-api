#include "ChannelSplitterNode.h"
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <algorithm>

namespace audioapi {

ChannelSplitterNode::ChannelSplitterNode(
    BaseAudioContext *context,
    unsigned numberOfOutputs)
    : AudioNode(context) {
  numberOfInputs_ = 1;
  numberOfOutputs_ = numberOfOutputs;

  channelCount_ = numberOfOutputs;
  channelCountMode_ = ChannelCountMode::EXPLICIT;
  channelInterpretation_ = ChannelInterpretation::DISCRETE;

  m_outputBuses.clear();
  for (unsigned i = 0; i < numberOfOutputs_; ++i) {
    m_outputBuses.push_back(
        std::make_shared<AudioBus>(
            RENDER_QUANTUM_SIZE, 1, context->getSampleRate()));
  }

  isInitialized_ = true;
}

void ChannelSplitterNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  if (inputBuses.empty() || !inputBuses[0]) {
    for (unsigned i = 0; i < numberOfOutputs_; ++i) {
      getOutputBus(i)->zero();
    }
    return;
  }

  const auto &sourceBus = inputBuses[0];
  unsigned numberOfSourceChannels = sourceBus->getNumberOfChannels();

  for (unsigned i = 0; i < numberOfOutputs_; ++i) {
    auto destinationBus = getOutputBus(i);

    if (i < numberOfSourceChannels) {
      destinationBus->getChannel(0)->copy(sourceBus->getChannel(i));
    } else {
      destinationBus->zero();
    }
  }
}

} // namespace audioapi