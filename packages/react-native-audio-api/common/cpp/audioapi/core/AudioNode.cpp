#include "AudioNode.h"
#include <audioapi/core/AudioParam.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/utils/AudioBus.h>
#include <stdexcept>
#include "NodeConnections.h"

// mostly wrong, left it as a template
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

AudioNode::~AudioNode() {
  if (isInitialized_) {
    cleanup();
  }
}

// --- Public API ---

void AudioNode::connect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      destination,
      outputIndex,
      inputIndex,
      AudioNodeManager::ConnectionType::CONNECT, );
}

void AudioNode::connect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  context_->getNodeManager()->addPendingParamConnection(
      shared_from_this(),
      param,
      outputIndex,
      AudioNodeManager::ConnectionType::CONNECT);
}

void AudioNode::disconnect() {
  m_connections->disconnectAll();
  // context_->getNodeManager()->addPendingNodeConnection(
  //     shared_from_this(),
  //     nullptr,
  //     0,
  //     0,
  //     AudioNodeManager::ConnectionType::DISCONNECT_ALL);
}

// disconnect a single output from any other nodes or params
void AudioNode::disconnect(unsigned int outputIndex) {
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      nullptr,
      outputIndex,
      0,
      AudioNodeManager::ConnectionType::DISCONNECT);
}

// disconnect all connections to a specific node param / node connections
void AudioNode::disconnect(const std::shared_ptr<AudioNode> &destination) {
  if (!destination)
    return;
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      destination,
      AudioNodeManager::ConnectionType::DISCONNECT,
      ALL_INDICES,
      ALL_INDICES);
}

void AudioNode::disconnect(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex) {
  if (!destination)
    return;
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      destination,
      AudioNodeManager::ConnectionType::DISCONNECT,
      outputIndex,
      ALL_INDICES);
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
  context_->getNodeManager()->addPendingNodeConnection(
      shared_from_this(),
      destination,
      AudioNodeManager::ConnectionType::DISCONNECT,
      outputIndex,
      inputIndex);
}

void AudioNode::disconnect(const std::shared_ptr<AudioParam> &param) {
  if (!param)
    return;
  // This would require a more detailed event or a new ConnectionType
  // For now, we can simplify and require an output index.
  // disconnect(param, 0);
}

void AudioNode::disconnect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  if (!param)
    return;
  if (outputIndex >= numberOfOutputs_)
    throw std::out_of_range("Output index out of bounds.");
  context_->getNodeManager()->addPendingParamConnection(
      shared_from_this(),
      param,
      AudioNodeManager::ConnectionType::DISCONNECT,
      outputIndex);
}

// --- Processing ---

std::shared_ptr<AudioBus> AudioNode::processAudio(
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  if (!isInitialized_)
    return getOutputBus(0);
  if (checkIsAlreadyProcessed && isAlreadyProcessed())
    return getOutputBus(0);

  const auto &processedInputs =
      m_connections->processAllInputs(framesToProcess, checkIsAlreadyProcessed);
  processNode(processedInputs, framesToProcess);

  return getOutputBus(0);
}

std::shared_ptr<AudioBus> AudioNode::getOutputBus(unsigned int outputIndex) {
  if (outputIndex < m_outputBuses.size()) {
    return m_outputBuses[outputIndex];
  }
  return nullptr;
}

// --- Getters & State ---
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
  return "max";
} // Placeholder
std::string AudioNode::getChannelInterpretation() const {
  return "speakers";
} // Placeholder

void AudioNode::enable() {
  if (isEnabled_)
    return;
  isEnabled_ = true;
  // This logic now needs to iterate through m_connections's outputs
  // m_connections->propagateEnable();
}

void AudioNode::disable() {
  if (!isEnabled_)
    return;
  isEnabled_ = false;
  // m_connections->propagateDisable();
}

// --- Handshake Methods ---

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
  m_connections->onInputConnected(source, outputIndexFromSource, inputIndex);
  if (source->isEnabled()) {
    onInputEnabled();
  }
}

void AudioNode::disconnectAll() {
  m_connections->disconnectAll();
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

// --- Processing & State ---

void AudioNode::processNode(
    const std::vector<std::shared_ptr<AudioBus>> &inputBuses,
    int framesToProcess) {
  // Default bridge implementation for backward compatibility
  auto firstInputBus = inputBuses.empty() ? nullptr : inputBuses[0];
  auto outputBus = getOutputBus(0);

  if (firstInputBus && outputBus) {
    if (outputBus.get() != firstInputBus.get()) {
      outputBus->copy(firstInputBus.get());
    }
  } else if (outputBus) {
    outputBus->zero();
  }

  // Call the old version
  processNode(outputBus, framesToProcess);
}

void AudioNode::processNode(
    std::shared_ptr<AudioBus> processingBus,
    int framesToProcess) {
  // Base implementation does nothing. To be overridden by simple nodes.
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
  m_connections->disconnectAll();
}

} // namespace audioapi