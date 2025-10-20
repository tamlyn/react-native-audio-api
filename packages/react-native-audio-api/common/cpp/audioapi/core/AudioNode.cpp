#include "AudioNode.h"
#include <audioapi/core/AudioParam.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/core/utils/NodeConnections.h>
#include <audioapi/utils/AudioBus.h>
#include <stdexcept>

namespace audioapi {

AudioNode::AudioNode(BaseAudioContext *context) : context_(context) {
  m_connections = std::make_unique<NodeConnections>(this, context);
  isInitialized_ = true;

  // Initialize output buses for a standard node
  m_outputBuses.resize(numberOfOutputs_);
  for (unsigned int i = 0; i < numberOfOutputs_; ++i) {
    m_outputBuses[i] = std::make_shared<AudioBus>(
        RENDER_QUANTUM_SIZE, channelCount_, context->getSampleRate());
  }
}

AudioNode::AudioNode(BaseAudioContext *context, unsigned int numberOfInputs)
    : context_(context), numberOfInputs_(numberOfInputs) {
  m_connections = std::make_unique<NodeConnections>(this, context);
  isInitialized_ = true;

  m_outputBuses.resize(numberOfOutputs_);
  for (unsigned int i = 0; i < numberOfOutputs_; ++i) {
    m_outputBuses[i] = std::make_shared<AudioBus>(
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
  m_connections->disconnect();
}

// disconnect a single output from any other nodes or params
void AudioNode::disconnect(unsigned int outputIndex) {
  // ask if we want to do index checks here or just in the ts layer
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  m_connections->disconnect(outputIndex);
}

// disconnect all connections to a specific node param / node connections
void AudioNode::disconnect(const std::shared_ptr<AudioNode> &destination) {
  if (!destination)
    return;
  m_connections->disconnect(destination);
}

void AudioNode::disconnect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex) {
  if (!destination)
    return;
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  m_connections->disconnect(destination, outputIndex);
}

void AudioNode::disconnect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  if (!destination)
    return;
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  if (inputIndex >= destination->getNumberOfInputs())
    throw std::out_of_range("Input index out of bounds.");
  m_connections->disconnect(destination, outputIndex, inputIndex);
}

void AudioNode::disconnect(const std::shared_ptr<AudioParam> &param) {
  if (!param)
    return;
  m_connections->disconnect(param);
}

void AudioNode::disconnect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  if (!param)
    return;
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  m_connections->disconnect(param, outputIndex);
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

  // this is very defensive, maybe too much
  if (m_outputBuses.size() != static_cast<size_t>(numberOfOutputs_)) {
    m_outputBuses.resize(numberOfOutputs_);
  }
  for (int i = 0; i < numberOfOutputs_; ++i) {
    if (!m_outputBuses[i]) {
      m_outputBuses[i] = std::make_shared<AudioBus>(
          static_cast<size_t>(framesToProcess),
          channelCount_,
          context_->getSampleRate());
    } else {
      if (m_outputBuses[i]->getSize() != static_cast<size_t>(framesToProcess) ||
          m_outputBuses[i]->getSampleRate() != context_->getSampleRate()) {
        m_outputBuses[i] = std::make_shared<AudioBus>(
            static_cast<size_t>(framesToProcess),
            channelCount_,
            context_->getSampleRate());
      }
      m_outputBuses[i]->zero();
    }
  }

  // we could also do the check in NodeConnections::processInputAtIndex, not
  // sure which is better
  if (!isEnabled_) {
    return;
  }

  // process all inputs for each input index
  const auto &inputBuses =
      m_connections->processAllInputs(framesToProcess, checkIsAlreadyProcessed);
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
  m_connections->propagateEnable();
}

void AudioNode::disable() {
  if (!isEnabled_)
    return;
  isEnabled_ = false;
  m_connections->propagateDisable();
}

void AudioNode::connectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  m_connections->connectNode(destination, outputIndex, inputIndex);
}

void AudioNode::onInputConnected(
    AudioNode *source,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!isInitialized_) {
    return;
  }

  m_connections->onInputConnected(source, outputIndexFromSource, inputIndex);
  if (source->isEnabled()) {
    onInputEnabled();
  }
}

void AudioNode::disconnectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  m_connections->disconnectNode(destination, outputIndex, inputIndex);
}

void AudioNode::onInputDisconnected(
    AudioNode *source,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!isInitialized_) {
    return;
  }

  m_connections->onInputDisconnected(source, outputIndexFromSource, inputIndex);
  if (source->isEnabled()) {
    onInputDisabled();
  }
}

void AudioNode::connectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  m_connections->connectParam(param, outputIndex);
}

void AudioNode::disconnectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  m_connections->disconnectParam(param, outputIndex);
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
  m_connections->cleanup();
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

  if (inputBuses.empty() || !inputBuses[0]) {
    processingBus = std::make_shared<AudioBus>(
        static_cast<size_t>(framesToProcess),
        channelCount_,
        context_->getSampleRate());
    processingBus->zero();
  } else {
    processingBus = inputBuses[0];
  }

  std::shared_ptr<AudioBus> returnedBus =
      processNode(processingBus, framesToProcess);

  if (!m_outputBuses.empty()) {
    auto outBus = getOutputBus(0);
    if (returnedBus && returnedBus != outBus) {
      outBus->copy(returnedBus.get());
    } else if (!returnedBus) {
      outBus->zero();
    }
  }
}

std::shared_ptr<AudioBus> AudioNode::getOutputBus(unsigned int index) {
  if (index >= m_outputBuses.size()) {
    throw std::out_of_range("Output index out of bounds.");
  }
  return m_outputBuses[index];
}

} // namespace audioapi