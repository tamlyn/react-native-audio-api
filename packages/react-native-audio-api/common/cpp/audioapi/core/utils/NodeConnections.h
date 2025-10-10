
#pragma once

#include <map>
#include <memory>
#include <vector>

namespace audioapi {

class AudioNode;
class AudioBus;
class AudioParam;
class BaseAudioContext;

// Describes an INCOMING connection from a source node's output port.
struct InputConnection {
  AudioNode *sourceNode;
  unsigned int outputIndexFromSource;
};

// Describes an OUTGOING connection to a destination node's input port.
struct OutputConnection {
  std::shared_ptr<AudioNode> destinationNode;
  unsigned int inputIndexAtDestination;
};

// Manages ALL connectivity (in and out) and input processing logic for an AudioNode.
class NodeConnections {
 public:
  explicit NodeConnections(AudioNode *owner, BaseAudioContext *context);

  // --- Public Disconnection Logic (called by AudioNode's public API) ---
  // disconnect all
  void disconnect();
  // disconnect everything (params and nodes) from an output index
  void disconnect(unsigned int outputIndex);
  // disconnects all outputs of the node which go to a specific  desitination       //Audio Node
  void disconnect(const std::shared_ptr<AudioNode> &node);
  // disconnects a specific output of the node from
  // any and all inputs of some destination node
  void disconnect(const std::shared_ptr<AudioNode> &node, unsigned int outputIndex);
  /*
Disconnects a specific output of the node from a specific input of some destination node
   */
  void disconnect(const std::shared_ptr<AudioNode> &node, unsigned int output, unsigned int input);
  /*
  Disconnects all outputs of the node that go to a specific destination node. The contribution of this node to the
  computed parameter value goes to 0 when this operation takes effect. The intrinsic parameter value is not affected
  by this operation.
  */
  void disconnect(const std::shared_ptr<AudioParam> &param);
  /*
  Disconnects a specific output of the AudioNode from a specific destination AudioParam. The contribution of this
  AudioNode to the computed parameter value goes to 0 when this operation takes effect. The intrinsic parameter
  value is not affected by this operation.
  */
  void disconnect(const std::shared_ptr<AudioParam> &param, unsigned int output);

  // --- Input Processing ---
  const std::vector<std::shared_ptr<AudioBus>> &processAllInputs(int framesToProcess, bool checkIsAlreadyProcessed);

  // --- Handshake Methods (called by AudioNode's protected methods) ---
  // adds a node to the output nodes map
  void connectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  // inserts a node into the inputs map
  void onInputConnected(AudioNode *sourceNode, unsigned int outputIndexFromSource, unsigned int inputIndex);
  // deletes all nodes from the output map
  void disconnectAll();
  // deletes the node from the output map
  void disconnectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  // deletes the node from the input map
  void onInputDisconnected(AudioNode *sourceNode, unsigned int outputIndexFromSource, unsigned int inputIndex);
  // adds param to the output param map
  void connectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);
  // deletes param from the output param map
  void disconnectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);
  // clears the output map and calls onInputDisconnected on nodes inside the map
  void cleanup();
  // Iterates through all outputs and calls onInputEnabled() on connected nodes.
  void propagateEnable();
  // Iterates through all outputs and calls onInputDisabled() on connected nodes.
  void propagateDisable();

 private:
  AudioNode *m_owner;
  BaseAudioContext *m_context;

  // --- Connection Maps ---
  std::map<unsigned int, std::vector<InputConnection>> m_indexedInputs;
  std::map<unsigned int, std::vector<OutputConnection>> m_indexedOutputs;
  std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>> m_indexedOutputParams;

  // used for calculations inside processnputAtIndex
  std::vector<std::shared_ptr<AudioBus>> m_processingInputBuses;

  // --- Internal Helpers ---
  std::shared_ptr<AudioBus> processInputAtIndex(unsigned int index, int framesToProcess, bool checkIsAlreadyProcessed);
  std::shared_ptr<AudioBus> applyChannelCountMode(const std::shared_ptr<AudioBus> &processingBus);
  void mixSourceBuses(const std::shared_ptr<AudioBus> &processingBus);
};

} // namespace audioapi
