#include <audioapi/core/AudioNode.h>
#include <audioapi/core/AudioParam.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/core/utils/NodeConnections.h>
#include <audioapi/utils/AudioBus.h>
#include <stdexcept>

namespace audioapi {

AudioNode::AudioNode(BaseAudioContext *context, unsigned int numberOfInputs)
    : context_(context), numberOfInputs_(numberOfInputs) {
  connections_ = std::make_unique<NodeConnections>(this, context);
  isInitialized_ = true;

  outputBuses_.resize(numberOfOutputs_);
  for (unsigned int i = 0; i < numberOfOutputs_; ++i) {
    outputBuses_[i] = std::make_shared<AudioBus>(
        RENDER_QUANTUM_SIZE, channelCount_, context->getSampleRate());
  }
}

AudioNode::~AudioNode() {
  if (isInitialized_) {
    cleanup();
  }
}

void AudioNode::connect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      destination,
      AudioNodeManager::ConnectionType::CONNECT,
      outputIndex,
      inputIndex);
}

void AudioNode::connect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  context_->getNodeManager()->addPendingParamConnection(
      shared_from_this(),
      param,
      AudioNodeManager::ConnectionType::CONNECT,
      outputIndex);
}

void AudioNode::disconnect() {
  connections_->disconnect();
}

// disconnect a single output from any other nodes or params
void AudioNode::disconnect(unsigned int outputIndex) {
  connections_->disconnect(outputIndex);
}

// disconnect all connections to a specific node param / node connections
bool AudioNode::disconnect(const std::shared_ptr<AudioNode> &destination) {
  if (destination == nullptr)
    return false;
  return connections_->disconnect(destination);
}

bool AudioNode::disconnect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex) {
  if (destination == nullptr)
    return false;
  return connections_->disconnect(destination, outputIndex);
}

bool AudioNode::disconnect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  if (destination == nullptr)
    return false;
  return connections_->disconnect(destination, outputIndex, inputIndex);
}

bool AudioNode::disconnect(const std::shared_ptr<AudioParam> &param) {
  if (param == nullptr)
    return false;
  return connections_->disconnect(param);
}

bool AudioNode::disconnect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  if (param == nullptr)
    return false;
  return connections_->disconnect(param, outputIndex);
}

void AudioNode::processAudio(
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  if (!isInitialized_ || framesToProcess <= 0) {
    return;
  }

  if (checkIsAlreadyProcessed && isAlreadyProcessed()) {
    return;
  }

  for (int i = 0; i < numberOfOutputs_; ++i) {
    outputBuses_[i]->zero();
  }

  // we could also do the check in NodeConnections::processInputAtIndex, not
  // sure which is better
  if (!isEnabled_) {
    return;
  }

  // process all inputs for each input index
  const auto &inputBuses =
      connections_->processAllInputs(framesToProcess, checkIsAlreadyProcessed);
  // process the node itself
  processNode(inputBuses, framesToProcess);
}

int AudioNode::getNumberOfInputs() const {
  return numberOfInputs_;
}
int AudioNode::getNumberOfOutputs() const {
  return numberOfOutputs_;
}
int AudioNode::getChannelCount() const {
  return channelCount_;
}
bool AudioNode::isEnabled() const {
  return isEnabled_;
}
std::string AudioNode::getChannelCountMode() const {
  return AudioNode::toString(channelCountMode_);
}
std::string AudioNode::getChannelInterpretation() const {
  return AudioNode::toString(channelInterpretation_);
}

std::string AudioNode::toString(ChannelCountMode mode) {
  switch (mode) {
    case ChannelCountMode::MAX:
      return "max";
    case ChannelCountMode::CLAMPED_MAX:
      return "clamped-max";
    case ChannelCountMode::EXPLICIT:
      return "explicit";
    default:
      throw std::invalid_argument("Unknown channel count mode");
  }
}

std::string AudioNode::toString(ChannelInterpretation interpretation) {
  switch (interpretation) {
    case ChannelInterpretation::SPEAKERS:
      return "speakers";
    case ChannelInterpretation::DISCRETE:
      return "discrete";
    default:
      throw std::invalid_argument("Unknown channel interpretation");
  }
}

void AudioNode::enable() {
  if (isEnabled_)
    return;
  isEnabled_ = true;
  connections_->propagateEnable();
}

void AudioNode::disable() {
  if (!isEnabled_)
    return;
  isEnabled_ = false;
  connections_->propagateDisable();
}

void AudioNode::connectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  connections_->connectNode(destination, outputIndex, inputIndex);
}

void AudioNode::onInputConnected(
    AudioNode *source,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!isInitialized_) {
    return;
  }

  connections_->onInputConnected(source, outputIndexFromSource, inputIndex);
  if (source->isEnabled()) {
    onInputEnabled();
  }
}

void AudioNode::disconnectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  connections_->disconnectNode(destination, outputIndex, inputIndex);
}

void AudioNode::onInputDisconnected(
    AudioNode *source,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!isInitialized_) {
    return;
  }

  connections_->onInputDisconnected(source, outputIndexFromSource, inputIndex);
  if (source->isEnabled()) {
    onInputDisabled();
  }
}

void AudioNode::connectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  connections_->connectParam(param, outputIndex);
}

void AudioNode::disconnectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  connections_->disconnectParam(param, outputIndex);
}

void AudioNode::onInputEnabled() {
  numberOfEnabledInputNodes_++;
  if (!isEnabled_) {
    enable();
  }
}

void AudioNode::onInputDisabled() {
  numberOfEnabledInputNodes_--;
  if (isEnabled_ && numberOfEnabledInputNodes_ == 0) {
    disable();
  }
}

bool AudioNode::isAlreadyProcessed() {
  std::size_t currentSampleFrame = context_->getCurrentSampleFrame();
  if (currentSampleFrame == lastRenderedFrame_) {
    return true;
  }
  lastRenderedFrame_ = currentSampleFrame;
  return false;
}

void AudioNode::cleanup() {
  isInitialized_ = false;
  connections_->cleanup();
}

std::shared_ptr<AudioBus> AudioNode::processNode(
    const std::shared_ptr<AudioBus> &processingBus,
    int framesToProcess) {
  if (!processingBus) {
    return nullptr;
  }
  processingBus->zero();
  return processingBus;
}

// this acts as an adapter for all the single input/output nodes (every node
// besides merger/splitter)
void AudioNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  std::shared_ptr<AudioBus> processingBus;

  // this handles source (0 input) node case
  if (inputBuses.empty() || inputBuses[0] == nullptr) {
    processingBus = getOutputBus(0);
    processingBus->zero();
  } else {
    processingBus = inputBuses[0];
  }

  std::shared_ptr<AudioBus> returnedBus =
      processNode(processingBus, framesToProcess);

  if (!outputBuses_.empty()) {
    auto outBus = getOutputBus(0);
    if (returnedBus && returnedBus != outBus) {
      outBus->copy(returnedBus.get());
    } else if (!returnedBus) {
      outBus->zero();
    }
  }
}

std::shared_ptr<AudioBus> AudioNode::getOutputBus(unsigned int index) {
  return outputBuses_[index];
}

} // namespace audioapi
