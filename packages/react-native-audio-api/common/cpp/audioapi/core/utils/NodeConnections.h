
#pragma once

#include <audioapi/core/utils/ConnectionTypes.h>

#include <map>
#include <memory>
#include <vector>

namespace audioapi {

class AudioNode;
class AudioBus;
class AudioParam;
class BaseAudioContext;

class NodeConnections {
 public:
  /// @brief Construct NodeConnections for a specific audio node
  /// @param owner The audio node that owns these connections
  /// @param context The audio context
  explicit NodeConnections(AudioNode *owner, BaseAudioContext *context);

  /// @brief Disconnect all connections from this node
  void disconnect();

  /// @brief Disconnect everything (params and nodes) from an output index
  /// @param outputIndex The output index to disconnect from
  void disconnect(unsigned int outputIndex);

  /// @brief Disconnects all outputs of the node which go to a specific destination
  /// @param node The destination node to disconnect from
  /// @return True if disconnection was successful, false otherwise
  bool disconnect(const std::shared_ptr<AudioNode> &node);

  /// @brief Disconnects a specific output of the node from any and all inputs of some destination node
  /// @param node The destination node to disconnect from
  /// @param outputIndex The output index to disconnect
  /// @return True if disconnection was successful, false otherwise
  bool disconnect(const std::shared_ptr<AudioNode> &node, unsigned int outputIndex);

  /// @brief Disconnects a specific output of the node from a specific input of some destination node
  /// @param node The destination node to disconnect from
  /// @param output The output index to disconnect
  /// @param input The input index to disconnect to
  /// @return True if disconnection was successful, false otherwise
  bool disconnect(const std::shared_ptr<AudioNode> &node, unsigned int output, unsigned int input);

  /// @brief Disconnects all outputs of the node that go to a specific destination node
  /// @note The contribution of this node to the computed parameter value goes to 0 when this operation takes effect
  /// @note The intrinsic parameter value is not affected by this operation
  /// @param param The destination AudioParam to disconnect from
  /// @return True if disconnection was successful, false otherwise
  bool disconnect(const std::shared_ptr<AudioParam> &param);

  /// @brief Disconnects a specific output of the AudioNode from a specific destination AudioParam
  /// @note The contribution of this AudioNode to the computed parameter value goes to 0 when this operation takes
  /// effect
  /// @note The intrinsic parameter value is not affected by this operation
  /// @param param The destination AudioParam to disconnect from
  /// @param output The output index to disconnect
  /// @return True if disconnection was successful, false otherwise
  bool disconnect(const std::shared_ptr<AudioParam> &param, unsigned int output);

  /// @brief Process all inputs and return the processed audio buses
  /// @param framesToProcess Number of frames to process
  /// @param checkIsAlreadyProcessed Whether to check if inputs are already processed
  /// @return Vector of processed audio buses
  const std::vector<std::shared_ptr<AudioBus>> &processAllInputs(int framesToProcess, bool checkIsAlreadyProcessed);

  /// @brief Adds a node to the output nodes map
  /// @param destination The destination node to connect to
  /// @param outputIndex The output index on this node
  /// @param inputIndex The input index on the destination node
  void connectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);

  /// @brief Inserts a node into the inputs map
  /// @param sourceNode The source node that connected
  /// @param outputIndexFromSource The output index from the source node
  /// @param inputIndex The input index on this node
  void onInputConnected(AudioNode *sourceNode, unsigned int outputIndexFromSource, unsigned int inputIndex);

  /// @brief Deletes the node from the output map
  /// @param destination The destination node to disconnect from
  /// @param outputIndex The output index on this node
  /// @param inputIndex The input index on the destination node
  void disconnectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);

  /// @brief Deletes the node from the input map
  /// @param sourceNode The source node that disconnected
  /// @param outputIndexFromSource The output index from the source node
  /// @param inputIndex The input index on this node
  void onInputDisconnected(AudioNode *sourceNode, unsigned int outputIndexFromSource, unsigned int inputIndex);

  /// @brief Adds param to the output param map
  /// @param param The AudioParam to connect
  /// @param outputIndex The output index to connect from
  void connectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  /// @brief Deletes param from the output param map
  /// @param param The AudioParam to disconnect
  /// @param outputIndex The output index to disconnect from
  void disconnectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  /// @brief Clears the output map and calls onInputDisconnected on nodes inside the map
  void cleanup();

  /// @brief Iterates through all outputs and calls onInputEnabled() on connected nodes
  void propagateEnable();

  /// @brief Iterates through all outputs and calls onInputDisabled() on connected nodes
  void propagateDisable();

 private:
  AudioNode *owner_;
  BaseAudioContext *context_;

  std::map<unsigned int, std::vector<InputConnection>> indexedInputs_;
  std::map<unsigned int, std::vector<OutputConnection>> indexedOutputs_;
  std::map<unsigned int, std::vector<std::shared_ptr<AudioParam>>> indexedOutputParams_;

  /// @brief Used for calculations inside processInputAtIndex
  std::vector<std::shared_ptr<AudioBus>> processingInputBuses_;
  std::shared_ptr<AudioBus> internalSummingBus_;

  /// @brief Processes all inputs connected to a particular input index
  /// @param index The input index to process
  /// @param framesToProcess Number of frames to process
  /// @param checkIsAlreadyProcessed Whether to check if inputs are already processed
  /// @return Processed AudioBus for the specified input
  std::shared_ptr<AudioBus> processInputAtIndex(unsigned int index, int framesToProcess, bool checkIsAlreadyProcessed);

  /// @brief Compute the number of channels for a specific input
  /// @param index The input index
  /// @return Number of channels for the input
  unsigned int computeNumberOfChannelsForInput(unsigned int index) const;

  /// @brief Helpers for processInputAtIndex
  /// @brief Get processing bus for a specific input index
  /// @param inputIndex The input index
  /// @param numChannels Number of channels needed
  /// @param framesToProcess Number of frames to process
  /// @return Processing bus for the specified index
  std::shared_ptr<AudioBus>
  getProcessingBusForIndex(unsigned int inputIndex, unsigned int numChannels, int framesToProcess);

  /// @brief Check if input is silent
  /// @param index The input index to check
  /// @return True if input is silent, false otherwise
  bool hasSilentInput(unsigned int index) const;

  /// @brief Create a silent bus for the specified input
  /// @param index The input index
  /// @param framesToProcess Number of frames to process
  /// @return Silent AudioBus
  std::shared_ptr<AudioBus> createSilentBus(unsigned int index, int framesToProcess);

  /// @brief Check if direct bus passthrough can be used
  /// @param connections The input connections to check
  /// @return True if direct passthrough can be used, false otherwise
  bool canUseDirectBusPassthrough(const std::vector<InputConnection> &connections) const;

  /// @brief Process a direct connection
  /// @param connection The input connection to process
  /// @param framesToProcess Number of frames to process
  /// @param checkIsAlreadyProcessed Whether to check if already processed
  /// @return Processed AudioBus
  std::shared_ptr<AudioBus>
  processDirectConnection(const InputConnection &connection, int framesToProcess, bool checkIsAlreadyProcessed);

  /// @brief Process multiple mixed connections
  /// @param index The input index
  /// @param connections The connections to process
  /// @param framesToProcess Number of frames to process
  /// @param checkIsAlreadyProcessed Whether to check if already processed
  /// @return Processed AudioBus
  std::shared_ptr<AudioBus> processMixedConnections(
      unsigned int index,
      const std::vector<InputConnection> &connections,
      int framesToProcess,
      bool checkIsAlreadyProcessed);

  /// @brief Find the maximum number of source channels
  /// @param connections The connections to check
  /// @return Maximum number of source channels
  unsigned int findMaxSourceChannels(const std::vector<InputConnection> &connections) const;

  /// @brief Safely get source bus from a node
  /// @param sourceNode The source node
  /// @param outputIndex The output index
  /// @return Source AudioBus
  std::shared_ptr<AudioBus> getSourceBusSafely(AudioNode *sourceNode, unsigned int outputIndex) const;

  /// @brief Prepare the summing bus for mixing
  /// @param computedChannels Number of computed channels
  /// @param maxSourceChannels Maximum number of source channels
  void prepareSummingBus(unsigned int computedChannels, unsigned int maxSourceChannels);

  /// @brief Sum all connections into the summing bus
  /// @param connections The connections to sum
  /// @param framesToProcess Number of frames to process
  /// @param checkIsAlreadyProcessed Whether to check if already processed
  void
  sumAllConnections(const std::vector<InputConnection> &connections, int framesToProcess, bool checkIsAlreadyProcessed);
};

} // namespace audioapi
