#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/ChannelMergerNode.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

namespace audioapi {

ChannelMergerNode::ChannelMergerNode(
    BaseAudioContext *context,
    unsigned int numberOfInputs)
    : AudioNode(context, numberOfInputs) {
  channelCount_ = 1;
  channelCountMode_ = ChannelCountMode::EXPLICIT;
  channelInterpretation_ = ChannelInterpretation::SPEAKERS;

  outputBuses_.clear();
  outputBuses_.push_back(
      std::make_shared<AudioBus>(
          RENDER_QUANTUM_SIZE, numberOfInputs_, context->getSampleRate()));

  isInitialized_ = true;
}

void ChannelMergerNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  auto outputBus = getOutputBus(0);

  size_t count_ = std::min(
      static_cast<size_t>(numberOfInputs_), inputBuses.size()); // cache size
  for (size_t i = 0; i < count_; ++i) {
    bool isInputConnected = inputBuses[i] != nullptr;
    if (isInputConnected) {
      const auto &sourceBus = inputBuses[i];
      outputBus->getChannel(i)->copy(sourceBus->getChannel(0));
    } else {
      outputBus->getChannel(i)->zero();
    }
  }
}

} // namespace audioapi
