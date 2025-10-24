#include <audioapi/core/AudioNode.h>
#include <audioapi/core/AudioParam.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/core/utils/NodeConnections.h>
#include <audioapi/utils/AudioBus.h>
#include <algorithm>
#include <stdexcept>

namespace audioapi {

NodeConnections::NodeConnections(AudioNode *owner, BaseAudioContext *context)
    : owner_(owner), context_(context) {
  processingInputBuses_.resize(owner_->getNumberOfInputs());
  internalSummingBus_ = std::make_shared<AudioBus>(
      RENDER_QUANTUM_SIZE, 1, context->getSampleRate());
}

void NodeConnections::disconnect() {
  // Disconnect from all nodes
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator it =
           indexedOutputs_.begin();
       it != indexedOutputs_.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      context_->getNodeManager()->addPendingNodeConnection(
          owner_->shared_from_this(),
          conn_it->destinationNode,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex,
          conn_it->inputIndexAtDestination);
    }
  }
  // Disconnect from all params
  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator it = indexedOutputParams_.begin();
       it != indexedOutputParams_.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      context_->getNodeManager()->addPendingParamConnection(
          owner_->shared_from_this(),
          *param_it,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex);
    }
  }
}

void NodeConnections::disconnect(unsigned int outputIndex) {
  // Disconnect all nodes from a specific output
  if (indexedOutputs_.count(outputIndex)) {
    std::vector<OutputConnection> &connections =
        indexedOutputs_.at(outputIndex);
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      context_->getNodeManager()->addPendingNodeConnection(
          owner_->shared_from_this(),
          conn_it->destinationNode,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex,
          conn_it->inputIndexAtDestination);
    }
  }

  // Disconnect all params from a specific output
  if (indexedOutputParams_.count(outputIndex)) {
    std::vector<std::shared_ptr<AudioParam>> &params =
        indexedOutputParams_.at(outputIndex);
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      context_->getNodeManager()->addPendingParamConnection(
          owner_->shared_from_this(),
          *param_it,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex);
    }
  }
}

bool NodeConnections::disconnect(const std::shared_ptr<AudioNode> &node) {
  bool disconnected = false;
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator it =
           indexedOutputs_.begin();
       it != indexedOutputs_.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      if (conn_it->destinationNode == node) {
        context_->getNodeManager()->addPendingNodeConnection(
            owner_->shared_from_this(),
            conn_it->destinationNode,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex,
            conn_it->inputIndexAtDestination);
        disconnected = true;
      }
    }
  }
  return disconnected;
}

bool NodeConnections::disconnect(
    const std::shared_ptr<AudioNode> &node,
    unsigned int outputIndex) {
  bool disconnected = false;
  if (indexedOutputs_.count(outputIndex)) {
    std::vector<OutputConnection> &connections =
        indexedOutputs_.at(outputIndex);
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      if (conn_it->destinationNode == node) {
        context_->getNodeManager()->addPendingNodeConnection(
            owner_->shared_from_this(),
            conn_it->destinationNode,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex,
            conn_it->inputIndexAtDestination);
        disconnected = true;
      }
    }
  }
  return disconnected;
}

bool NodeConnections::disconnect(
    const std::shared_ptr<AudioNode> &node,
    unsigned int output,
    unsigned int input) {
  bool connectionFound = false;
  if (indexedOutputs_.count(output)) {
    auto &connections = indexedOutputs_.at(output);
    for (const auto &conn : connections) {
      if (conn.destinationNode == node &&
          conn.inputIndexAtDestination == input) {
        connectionFound = true;
        break;
      }
    }
  }
  // This is a specific request, so we can dispatch a single event.
  context_->getNodeManager()->addPendingNodeConnection(
      owner_->shared_from_this(),
      node,
      AudioNodeManager::ConnectionType::DISCONNECT,
      output,
      input);
  return connectionFound;
}

bool NodeConnections::disconnect(const std::shared_ptr<AudioParam> &param) {
  bool disconnected = false;
  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator it = indexedOutputParams_.begin();
       it != indexedOutputParams_.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      if (*param_it == param) {
        context_->getNodeManager()->addPendingParamConnection(
            owner_->shared_from_this(),
            *param_it,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex);
        disconnected = true;
      }
    }
  }
  return disconnected;
}

bool NodeConnections::disconnect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int output) {
  bool connectionFound = false;
  if (indexedOutputParams_.count(output)) {
    auto &params = indexedOutputParams_.at(output);
    for (const auto &p : params) {
      if (p == param) {
        connectionFound = true;
        break;
      }
    }
  }
  if (connectionFound) {
    context_->getNodeManager()->addPendingParamConnection(
        owner_->shared_from_this(),
        param,
        AudioNodeManager::ConnectionType::DISCONNECT,
        output);
  }
  return connectionFound;
}

void NodeConnections::connectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  OutputConnection newConnection{destination, inputIndex};
  indexedOutputs_[outputIndex].push_back(newConnection);
  destination->onInputConnected(owner_, outputIndex, inputIndex);
}

void NodeConnections::onInputConnected(
    AudioNode *sourceNode,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  InputConnection newConnection{sourceNode, outputIndexFromSource};
  indexedInputs_[inputIndex].push_back(newConnection);
}

void NodeConnections::disconnectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  if (!indexedOutputs_.count(outputIndex)) {
    return;
  }
  auto &connections = indexedOutputs_.at(outputIndex);
  auto it = std::remove_if(
      connections.begin(),
      connections.end(),
      [&](const OutputConnection &conn) {
        return conn.destinationNode == destination &&
            conn.inputIndexAtDestination == inputIndex;
      });
  if (it != connections.end()) {
    connections.erase(it, connections.end());
    destination->onInputDisconnected(owner_, outputIndex, inputIndex);
  }
}

void NodeConnections::onInputDisconnected(
    AudioNode *sourceNode,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!indexedInputs_.count(inputIndex)) {
    return;
  }
  auto &connections = indexedInputs_.at(inputIndex);
  auto it = std::remove_if(
      connections.begin(), connections.end(), [&](const InputConnection &conn) {
        return conn.sourceNode == sourceNode &&
            conn.outputIndexFromSource == outputIndexFromSource;
      });
  if (it != connections.end()) {
    connections.erase(it, connections.end());
  }
}

void NodeConnections::connectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  indexedOutputParams_[outputIndex].push_back(param);
  param->addInputNode(owner_, outputIndex);
}

void NodeConnections::disconnectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  if (!indexedOutputParams_.count(outputIndex))
    return;
  auto &params = indexedOutputParams_.at(outputIndex);
  auto it = std::remove(params.begin(), params.end(), param);
  if (it != params.end()) {
    param->removeInputNode(owner_, outputIndex);
    params.erase(it, params.end());
  }
}

void NodeConnections::cleanup() {
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator map_it =
           indexedOutputs_.begin();
       map_it != indexedOutputs_.end();
       ++map_it) {
    unsigned int outputIndex = map_it->first;
    std::vector<OutputConnection> &connections = map_it->second;
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      const std::shared_ptr<AudioNode> &destinationNode =
          conn_it->destinationNode;
      unsigned int inputIndexAtDestination = conn_it->inputIndexAtDestination;
      if (destinationNode) {
        destinationNode->onInputDisconnected(
            owner_, outputIndex, inputIndexAtDestination);
      }
    }
  }

  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator map_it = indexedOutputParams_.begin();
       map_it != indexedOutputParams_.end();
       ++map_it) {
    unsigned int outputIndex = map_it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = map_it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      const std::shared_ptr<AudioParam> &destinationParam = *param_it;
      if (destinationParam) {
        destinationParam->removeInputNode(owner_, outputIndex);
      }
    }
  }

  indexedInputs_.clear();
  indexedOutputs_.clear();
  indexedOutputParams_.clear();
}

void NodeConnections::propagateEnable() {
  for (std::map<unsigned int, std::vector<OutputConnection>>::const_iterator
           it = indexedOutputs_.begin();
       it != indexedOutputs_.end();
       ++it) {
    const std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::const_iterator conn_it =
             connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      conn_it->destinationNode->onInputEnabled();
    }
  }
}

void NodeConnections::propagateDisable() {
  for (std::map<unsigned int, std::vector<OutputConnection>>::const_iterator
           it = indexedOutputs_.begin();
       it != indexedOutputs_.end();
       ++it) {
    const std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::const_iterator conn_it =
             connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      conn_it->destinationNode->onInputDisabled();
    }
  }
}

unsigned int NodeConnections::computeNumberOfChannelsForInput(
    unsigned int index) const {
  const ChannelCountMode mode = owner_->getChannelCountModeEnum();

  if (mode == ChannelCountMode::EXPLICIT) {
    return owner_->getChannelCount();
  }

  unsigned int maxInputChannels = 0;
  if (indexedInputs_.count(index)) {
    const std::vector<InputConnection> &connections = indexedInputs_.at(index);
    for (const InputConnection &ic : connections) {
      if (ic.sourceNode) {
        auto sourceBus = ic.sourceNode->getOutputBus(ic.outputIndexFromSource);
        maxInputChannels = std::max(
            maxInputChannels, (unsigned int)sourceBus->getNumberOfChannels());
      }
    }
  }

  unsigned int computedChannels = 0;
  if (mode == ChannelCountMode::MAX) {
    computedChannels = maxInputChannels;
  } else { // CLAMPED_MAX
    computedChannels =
        std::min((unsigned int)owner_->getChannelCount(), maxInputChannels);
  }

  return computedChannels > 0 ? computedChannels : 1;
}

const std::vector<std::shared_ptr<AudioBus>> &NodeConnections::processAllInputs(
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  int numInputs = owner_->getNumberOfInputs();

  if (internalSummingBus_->getSampleRate() != context_->getSampleRate()) {
    internalSummingBus_ = std::make_shared<AudioBus>(
        RENDER_QUANTUM_SIZE, 1, context_->getSampleRate());
  }

  for (int i = 0; i < numInputs; ++i) {
    processingInputBuses_[i] = processInputAtIndex(
        static_cast<unsigned int>(i), framesToProcess, checkIsAlreadyProcessed);
  }

  return processingInputBuses_;
}

std::shared_ptr<AudioBus> NodeConnections::processInputAtIndex(
    unsigned int index,
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  if (hasSilentInput(index)) {
    return createSilentBus(index, framesToProcess);
  }

  const std::vector<InputConnection> &connections = indexedInputs_.at(index);

  if (canUseDirectBusPassthrough(connections)) {
    return processDirectConnection(
        connections[0], framesToProcess, checkIsAlreadyProcessed);
  }

  return processMixedConnections(
      index, connections, framesToProcess, checkIsAlreadyProcessed);
}

bool NodeConnections::hasSilentInput(unsigned int index) const {
  return !indexedInputs_.count(index) || indexedInputs_.at(index).empty();
}

std::shared_ptr<AudioBus> NodeConnections::createSilentBus(
    unsigned int index,
    int framesToProcess) {
  unsigned int channels = owner_->getChannelCount();
  auto bus = getProcessingBusForIndex(index, channels, framesToProcess);
  bus->zero();
  return bus;
}

bool NodeConnections::canUseDirectBusPassthrough(
    const std::vector<InputConnection> &connections) const {
  return connections.size() == 1 &&
      owner_->getChannelCountModeEnum() == ChannelCountMode::MAX;
}

std::shared_ptr<AudioBus> NodeConnections::processDirectConnection(
    const InputConnection &connection,
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  AudioNode *sourceNode = connection.sourceNode;
  if (sourceNode) {
    sourceNode->processAudio(framesToProcess, checkIsAlreadyProcessed);
    return sourceNode->getOutputBus(connection.outputIndexFromSource);
  }

  // Fallback if source is null
  auto fallback = getProcessingBusForIndex(0, 1, framesToProcess);
  fallback->zero();
  return fallback;
}

std::shared_ptr<AudioBus> NodeConnections::processMixedConnections(
    unsigned int index,
    const std::vector<InputConnection> &connections,
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  unsigned int computedChannels = computeNumberOfChannelsForInput(index);
  unsigned int maxSourceChannels = findMaxSourceChannels(connections);

  prepareSummingBus(computedChannels, maxSourceChannels);
  internalSummingBus_->zero();

  sumAllConnections(connections, framesToProcess, checkIsAlreadyProcessed);

  auto finalBus =
      getProcessingBusForIndex(index, computedChannels, framesToProcess);
  finalBus->copy(internalSummingBus_.get(), 0, 0, framesToProcess);

  return finalBus;
}

unsigned int NodeConnections::findMaxSourceChannels(
    const std::vector<InputConnection> &connections) const {
  unsigned int maxChannels = 0;

  for (const InputConnection &ic : connections) {
    if (!ic.sourceNode)
      continue;

    auto srcBus = getSourceBusSafely(ic.sourceNode, ic.outputIndexFromSource);
    if (srcBus) {
      maxChannels =
          std::max(maxChannels, (unsigned int)srcBus->getNumberOfChannels());
    }
  }

  return maxChannels;
}

std::shared_ptr<AudioBus> NodeConnections::getSourceBusSafely(
    AudioNode *sourceNode,
    unsigned int outputIndex) const {
  try {
    return sourceNode->getOutputBus(outputIndex);
  } catch (...) {
    return nullptr;
  }
}

void NodeConnections::prepareSummingBus(
    unsigned int computedChannels,
    unsigned int maxSourceChannels) {
  unsigned int requiredChannels = std::max(computedChannels, maxSourceChannels);
  if (requiredChannels == 0) {
    requiredChannels = 1;
  }

  bool needsReallocation = internalSummingBus_ == nullptr ||
      internalSummingBus_->getNumberOfChannels() != requiredChannels ||
      internalSummingBus_->getSampleRate() != context_->getSampleRate();

  if (needsReallocation) {
    internalSummingBus_ = std::make_shared<AudioBus>(
        RENDER_QUANTUM_SIZE, requiredChannels, context_->getSampleRate());
  }
}

void NodeConnections::sumAllConnections(
    const std::vector<InputConnection> &connections,
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  for (const InputConnection &ic : connections) {
    if (!ic.sourceNode)
      continue;

    ic.sourceNode->processAudio(framesToProcess, checkIsAlreadyProcessed);

    auto srcBus = getSourceBusSafely(ic.sourceNode, ic.outputIndexFromSource);
    if (srcBus) {
      internalSummingBus_->sum(
          srcBus.get(), owner_->getChannelInterpretationEnum());
    }
  }
}

std::shared_ptr<AudioBus> NodeConnections::getProcessingBusForIndex(
    unsigned int inputIndex,
    unsigned int numChannels,
    int framesToProcess) {
  auto &bus = processingInputBuses_[inputIndex];
  // this handles source (0 input) node case
  if (bus == nullptr || bus->getNumberOfChannels() != numChannels ||
      bus->getSampleRate() != context_->getSampleRate()) {
    bus = std::make_shared<AudioBus>(
        RENDER_QUANTUM_SIZE, numChannels, context_->getSampleRate());
  }
  return bus;
}

} // namespace audioapi
