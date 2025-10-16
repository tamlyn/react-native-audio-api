
#pragma once

#include <audioapi/core/types/ChannelCountMode.h>
#include <audioapi/core/types/ChannelInterpretation.h>
#include <audioapi/core/utils/Constants.h>

#include <limits>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace audioapi {

class AudioBus;
class BaseAudioContext;
class AudioParam;
class NodeConnections;

class AudioNode : public std::enable_shared_from_this<AudioNode> {
 public:
  // allocates correct amount of space for m_outputBuses
  explicit AudioNode(BaseAudioContext *context);
  virtual ~AudioNode();

  // just delegates it to the NodeManager
  void
  connect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex = 0, unsigned int inputIndex = 0);
  // just delegates it to the NodeManager
  void connect(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex = 0);

  // Overloaded Disconnect Methode - these just call equivalent methods in NodeConnection
  void disconnect();
  void disconnect(unsigned int outputIndex);
  void disconnect(const std::shared_ptr<AudioNode> &destination);
  void disconnect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex);
  void disconnect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  void disconnect(const std::shared_ptr<AudioParam> &param);
  void disconnect(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  // calls NodeConnections::processAllInputs
  virtual std::shared_ptr<AudioBus> processAudio(int framesToProcess, bool checkIsAlreadyProcessed);

  std::shared_ptr<AudioBus> getOutputBus(unsigned int outputIndex);

  int getNumberOfInputs() const;
  int getNumberOfOutputs() const;
  int getChannelCount() const;
  bool isEnabled() const;
  std::string getChannelCountMode() const;
  std::string getChannelInterpretation() const;
  BaseAudioContext *getContext() const {
    return context_;
  }
  ChannelCountMode getChannelCountModeEnum() const {
    return channelCountMode_;
  }
  ChannelInterpretation getChannelInterpretationEnum() const {
    return channelInterpretation_;
  }
  // Sets the node's state to enabled and propagates this change to its outputs.
  void enable();
  // Sets the node's state to disabled and propagates this change to its outputs.
  virtual void disable();

 protected:
  friend class AudioNodeManager;
  friend class NodeConnections;

  // these are forwarders to NodeConnections
  void connectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  void onInputConnected(AudioNode *source, unsigned int outputIndexFromSource, unsigned int inputIndex);

  void disconnectAll();
  void disconnectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  void onInputDisconnected(AudioNode *source, unsigned int outputIndexFromSource, unsigned int inputIndex);

  void connectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);
  void disconnectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  // created for backwards compatibility
  virtual std::shared_ptr<AudioBus> processNode(const std::shared_ptr<AudioBus> &processingBus, int framesToProcess);
  virtual void processNode(const std::vector<std::shared_ptr<AudioBus>> &inputBuses, int framesToProcess);

  // Called by an upstream node when it becomes enabled. Increments the active input counter.
  void onInputEnabled();
  // Called by an upstream node when it becomes disabled. Decrements the active input counter.
  void onInputDisabled();

  BaseAudioContext *context_;
  int numberOfInputs_ = 1;
  int numberOfOutputs_ = 1;
  int channelCount_ = 2;
  ChannelCountMode channelCountMode_ = ChannelCountMode::MAX;
  ChannelInterpretation channelInterpretation_ = ChannelInterpretation::SPEAKERS;

  std::vector<std::shared_ptr<AudioBus>> m_outputBuses;
  std::unique_ptr<NodeConnections> m_connections;

  int numberOfEnabledInputNodes_ = 0;
  bool isInitialized_ = false;
  bool isEnabled_ = true;
  std::size_t lastRenderedFrame_{SIZE_MAX};

 private:
  bool isAlreadyProcessed();
  static std::string toString(ChannelCountMode mode);
  static std::string toString(ChannelInterpretation interpretation);
  void cleanup();
};

} // namespace audioapi
