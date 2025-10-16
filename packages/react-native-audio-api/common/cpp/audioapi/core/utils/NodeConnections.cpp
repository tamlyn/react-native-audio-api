#include "NodeConnections.h"
#include <audioapi/core/AudioNode.h>
#include <audioapi/core/AudioParam.h>
#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/utils/AudioNodeManager.h>
#include <audioapi/utils/AudioBus.h>
#include <algorithm>
#include <stdexcept>

namespace audioapi {

NodeConnections::NodeConnections(AudioNode *owner, BaseAudioContext *context)
    : m_owner(owner), m_context(context) {}

void NodeConnections::disconnect() {
  // Disconnect from all nodes
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator it =
           m_indexedOutputs.begin();
       it != m_indexedOutputs.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      m_context->getNodeManager()->addPendingNodeConnection(
          m_owner->shared_from_this(),
          conn_it->destinationNode,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex,
          conn_it->inputIndexAtDestination);
    }
  }

  // Disconnect from all params
  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator it = m_indexedOutputParams.begin();
       it != m_indexedOutputParams.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      m_context->getNodeManager()->addPendingParamConnection(
          m_owner->shared_from_this(),
          *param_it,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex);
    }
  }
}

void NodeConnections::disconnect(unsigned int outputIndex) {
  // Disconnect all nodes from a specific output
  if (m_indexedOutputs.count(outputIndex)) {
    std::vector<OutputConnection> &connections =
        m_indexedOutputs.at(outputIndex);
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      m_context->getNodeManager()->addPendingNodeConnection(
          m_owner->shared_from_this(),
          conn_it->destinationNode,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex,
          conn_it->inputIndexAtDestination);
    }
  }

  // Disconnect all params from a specific output
  if (m_indexedOutputParams.count(outputIndex)) {
    std::vector<std::shared_ptr<AudioParam>> &params =
        m_indexedOutputParams.at(outputIndex);
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      m_context->getNodeManager()->addPendingParamConnection(
          m_owner->shared_from_this(),
          *param_it,
          AudioNodeManager::ConnectionType::DISCONNECT,
          outputIndex);
    }
  }
}

void NodeConnections::disconnect(const std::shared_ptr<AudioNode> &node) {
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator it =
           m_indexedOutputs.begin();
       it != m_indexedOutputs.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      if (conn_it->destinationNode == node) {
        m_context->getNodeManager()->addPendingNodeConnection(
            m_owner->shared_from_this(),
            conn_it->destinationNode,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex,
            conn_it->inputIndexAtDestination);
      }
    }
  }
}

void NodeConnections::disconnect(
    const std::shared_ptr<AudioNode> &node,
    unsigned int outputIndex) {
  if (m_indexedOutputs.count(outputIndex)) {
    std::vector<OutputConnection> &connections =
        m_indexedOutputs.at(outputIndex);
    for (std::vector<OutputConnection>::iterator conn_it = connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      if (conn_it->destinationNode == node) {
        m_context->getNodeManager()->addPendingNodeConnection(
            m_owner->shared_from_this(),
            conn_it->destinationNode,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex,
            conn_it->inputIndexAtDestination);
      }
    }
  }
}

void NodeConnections::disconnect(
    const std::shared_ptr<AudioNode> &node,
    unsigned int output,
    unsigned int input) {
  // This is a specific request, so we can dispatch a single event.
  m_context->getNodeManager()->addPendingNodeConnection(
      m_owner->shared_from_this(),
      node,
      AudioNodeManager::ConnectionType::DISCONNECT,
      output,
      input);
}

void NodeConnections::disconnect(const std::shared_ptr<AudioParam> &param) {
  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator it = m_indexedOutputParams.begin();
       it != m_indexedOutputParams.end();
       ++it) {
    unsigned int outputIndex = it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      if (*param_it == param) {
        m_context->getNodeManager()->addPendingParamConnection(
            m_owner->shared_from_this(),
            *param_it,
            AudioNodeManager::ConnectionType::DISCONNECT,
            outputIndex);
      }
    }
  }
}

void NodeConnections::disconnect(
    const std::shared_ptr<AudioParam> &param,
    unsigned int output) {
  m_context->getNodeManager()->addPendingParamConnection(
      m_owner->shared_from_this(),
      param,
      AudioNodeManager::ConnectionType::DISCONNECT,
      output);
}

void NodeConnections::connectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  OutputConnection newConnection{destination, inputIndex};
  m_indexedOutputs[outputIndex].push_back(newConnection);
  destination->onInputConnected(m_owner, outputIndex, inputIndex);
}

void NodeConnections::onInputConnected(
    AudioNode *sourceNode,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  InputConnection newConnection{sourceNode, outputIndexFromSource};
  m_indexedInputs[inputIndex].push_back(newConnection);
}

// not sure if this is needed/valid - probably should be handled in cleanup()
void NodeConnections::disconnectAll() {
  // This internal method is called on cleanup. It should directly notify nodes.
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator it =
           m_indexedOutputs.begin();
       it != m_indexedOutputs.end();
       ++it) {
    const unsigned int outputIndex = it->first;
    const std::vector<OutputConnection> &connections = it->second;
    for (std::vector<OutputConnection>::const_iterator conn_it =
             connections.begin();
         conn_it != connections.end();
         ++conn_it) {
      conn_it->destinationNode->onInputDisconnected(
          m_owner, outputIndex, conn_it->inputIndexAtDestination);
    }
  }
  m_indexedOutputs.clear();

  m_indexedOutputParams.clear();
}

void NodeConnections::disconnectNode(
    const std::shared_ptr<AudioNode> &destination,
    unsigned int outputIndex,
    unsigned int inputIndex) {
  if (!m_indexedOutputs.count(outputIndex)) {
    return;
  }
  auto &connections = m_indexedOutputs.at(outputIndex);
  auto it = std::remove_if(
      connections.begin(),
      connections.end(),
      [&](const OutputConnection &conn) {
        return conn.destinationNode == destination &&
            conn.inputIndexAtDestination == inputIndex;
      });
  if (it != connections.end()) {
    connections.erase(it, connections.end());
    destination->onInputDisconnected(m_owner, outputIndex, inputIndex);
  }
}

void NodeConnections::onInputDisconnected(
    AudioNode *sourceNode,
    unsigned int outputIndexFromSource,
    unsigned int inputIndex) {
  if (!m_indexedInputs.count(inputIndex)) {
    return;
  }
  auto &connections = m_indexedInputs.at(inputIndex);
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
  m_indexedOutputParams[outputIndex].push_back(param);
  param->addInputNode(m_owner);
}

void NodeConnections::disconnectParam(
    const std::shared_ptr<AudioParam> &param,
    unsigned int outputIndex) {
  if (!m_indexedOutputParams.count(outputIndex))
    return;
  auto &params = m_indexedOutputParams.at(outputIndex);
  auto it = std::remove(params.begin(), params.end(), param);
  if (it != params.end()) {
    param->removeInputNode(m_owner);
    params.erase(it, params.end());
  }
}

// og version only notified the output nodes and ignored params
void NodeConnections::cleanup() {
  for (std::map<unsigned int, std::vector<OutputConnection>>::iterator map_it =
           m_indexedOutputs.begin();
       map_it != m_indexedOutputs.end();
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
            m_owner, outputIndex, inputIndexAtDestination);
      }
    }
  }

  for (std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>>::
           iterator map_it = m_indexedOutputParams.begin();
       map_it != m_indexedOutputParams.end();
       ++map_it) {
    unsigned int outputIndex = map_it->first;
    std::vector<std::shared_ptr<AudioParam>> &params = map_it->second;
    for (std::vector<std::shared_ptr<AudioParam>>::iterator param_it =
             params.begin();
         param_it != params.end();
         ++param_it) {
      const std::shared_ptr<AudioParam> &destinationParam = *param_it;
      if (destinationParam) {
        destinationParam->removeInputNode(m_owner);
      }
    }
  }

  m_indexedInputs.clear();
  m_indexedOutputs.clear();
  m_indexedOutputParams.clear();
}

void NodeConnections::propagateEnable() {
  for (std::map<unsigned int, std::vector<OutputConnection>>::const_iterator
           it = m_indexedOutputs.begin();
       it != m_indexedOutputs.end();
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
           it = m_indexedOutputs.begin();
       it != m_indexedOutputs.end();
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
  const ChannelCountMode mode = m_owner->getChannelCountModeEnum();

  if (mode == ChannelCountMode::EXPLICIT) {
    return m_owner->getChannelCount();
  }

  unsigned int maxInputChannels = 0;
  if (m_indexedInputs.count(index)) {
    const std::vector<InputConnection> &connections = m_indexedInputs.at(index);
    for (const InputConnection &ic : connections) {
      if (ic.sourceNode) {
        maxInputChannels = std::max(
            maxInputChannels, (unsigned int)ic.sourceNode->getChannelCount());
      }
    }
  }

  unsigned int computedChannels = 0;
  if (mode == ChannelCountMode::MAX) {
    computedChannels = maxInputChannels;
  } else { // CLAMPED_MAX
    computedChannels =
        std::min((unsigned int)m_owner->getChannelCount(), maxInputChannels);
  }

  return computedChannels > 0 ? computedChannels : 1;
}

const std::vector<std::shared_ptr<AudioBus>> &NodeConnections::processAllInputs(
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  int numInputs = m_owner->getNumberOfInputs();

  m_processingInputBuses.resize(static_cast<size_t>(numInputs));

  for (int i = 0; i < numInputs; ++i) {
    m_processingInputBuses[i] = processInputAtIndex(
        static_cast<unsigned int>(i), framesToProcess, checkIsAlreadyProcessed);
  }

  return m_processingInputBuses;
}

std::shared_ptr<AudioBus> NodeConnections::processInputAtIndex(
    unsigned int index,
    int framesToProcess,
    bool checkIsAlreadyProcessed) {
  if (!m_indexedInputs.count(index) || m_indexedInputs.at(index).empty()) {
    unsigned int channels = m_owner->getChannelCount();
    auto bus = getProcessingBusForIndex(index, channels, framesToProcess);
    bus->zero();
    return bus;
  }

  const std::vector<InputConnection> &connections = m_indexedInputs.at(index);
  const ChannelCountMode mode = m_owner->getChannelCountModeEnum();

  if (connections.size() == 1 && mode == ChannelCountMode::MAX) {
    AudioNode *sourceNode = connections[0].sourceNode;
    if (sourceNode) {
      sourceNode->processAudio(framesToProcess, checkIsAlreadyProcessed);
      try {
        return sourceNode->getOutputBus(connections[0].outputIndexFromSource);
      } catch (...) {
      }
    }
    // fallback to silent bus
    auto fallback = getProcessingBusForIndex(index, 1, framesToProcess);
    fallback->zero();
    return fallback;
  }

  unsigned int computedChannels = computeNumberOfChannelsForInput(index);
  auto sumBus =
      getProcessingBusForIndex(index, computedChannels, framesToProcess);

  sumBus->zero();

  for (const InputConnection &ic : connections) {
    AudioNode *sourceNode = ic.sourceNode;
    if (!sourceNode)
      continue;

    sourceNode->processAudio(framesToProcess, checkIsAlreadyProcessed);

    std::shared_ptr<AudioBus> srcBus;
    try {
      srcBus = sourceNode->getOutputBus(ic.outputIndexFromSource);
    } catch (...) {
      srcBus = nullptr;
    }

    if (srcBus) {
      sumBus->sum(srcBus.get(), m_owner->getChannelInterpretationEnum());
    }
  }

  return sumBus;
}

std::shared_ptr<AudioBus> NodeConnections::getProcessingBusForIndex(
    unsigned int inputIndex,
    unsigned int numChannels,
    int framesToProcess) {
  if (m_processingInputBuses.size() <= inputIndex) {
    m_processingInputBuses.resize(inputIndex + 1);
  }

  auto &bus = m_processingInputBuses[inputIndex];
  if (!bus || bus->getSize() != static_cast<size_t>(framesToProcess) ||
      bus->getNumberOfChannels() != numChannels ||
      bus->getSampleRate() != m_context->getSampleRate()) {
    bus = std::make_shared<AudioBus>(
        static_cast<size_t>(framesToProcess),
        numChannels,
        m_context->getSampleRate());
  }
  return bus;
}

} // namespace audioapi
