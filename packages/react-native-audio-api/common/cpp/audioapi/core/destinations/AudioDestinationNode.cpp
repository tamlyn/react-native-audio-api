#include <audioapi/core/AudioNode.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/destinations/AudioDestinationNode.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/core/utils/NodeConnections.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

AudioDestinationNode::AudioDestinationNode(BaseAudioContext *context)
    : AudioNode(context), currentSampleFrame_(0) {
  numberOfOutputs_ = 0;
  numberOfInputs_ = 1;
  channelCountMode_ = ChannelCountMode::EXPLICIT;
  isInitialized_ = true;
}

std::size_t AudioDestinationNode::getCurrentSampleFrame() const {
  return currentSampleFrame_;
}

double AudioDestinationNode::getCurrentTime() const {
  return static_cast<double>(currentSampleFrame_) / context_->getSampleRate();
}

void AudioDestinationNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  if (inputBuses.empty() || !inputBuses[0]) {
    return;
  }

  getOutputBus(0)->copy(inputBuses[0].get());
}

void AudioDestinationNode::renderAudio(
    const std::shared_ptr<AudioBus> &destinationBus,
    int numFrames) {
  if (numFrames <= 0 || !destinationBus || !isInitialized_) {
    return;
  }

  context_->getNodeManager()->preProcessGraph();

  destinationBus->zero();

  const auto &inputBuses = m_connections->processAllInputs(numFrames, true);

  if (!inputBuses.empty() && inputBuses[0]) {
    destinationBus->copy(inputBuses[0].get());
  }

  destinationBus->normalize();

  currentSampleFrame_ += numFrames;
}

} // namespace audioapi
