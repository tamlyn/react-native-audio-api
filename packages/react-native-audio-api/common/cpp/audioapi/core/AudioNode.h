
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
  //explicit AudioNode(BaseAudioContext *context);
  virtual ~AudioNode();

  // just delegates it to the NodeManager
  void
  connect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex = 0, unsigned int inputIndex = 0);
  // just delegates it to the NodeManager
  void connect(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex = 0);

  // overloaded disconnect methods - these just call equivalent methods in NodeConnection
  void disconnect();
  void disconnect(unsigned int outputIndex);
  bool disconnect(const std::shared_ptr<AudioNode> &destination);
  bool disconnect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex);
  bool disconnect(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  bool disconnect(const std::shared_ptr<AudioParam> &param);
  bool disconnect(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  // calls NodeConnections::processAllInputs
  virtual void processAudio(int framesToProcess, bool checkIsAlreadyProcessed);

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
  // sets the node's state to enabled and propagates this change to its outputs.
  void enable();
  // sets the node's state to disabled and propagates this change to its outputs.
  virtual void disable();

 protected:
  friend class AudioNodeManager;
  friend class NodeConnections;
  friend class ConvolverNode;
  friend class AudioDestinationNode;

  explicit AudioNode(BaseAudioContext *context, unsigned int numberOfInputs = DEFAULT_NUMBER_OF_INPUTS);

  // these are all forwarders to NodeConnections
  void connectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  void onInputConnected(AudioNode *source, unsigned int outputIndexFromSource, unsigned int inputIndex);

  void disconnectNode(const std::shared_ptr<AudioNode> &destination, unsigned int outputIndex, unsigned int inputIndex);
  void onInputDisconnected(AudioNode *source, unsigned int outputIndexFromSource, unsigned int inputIndex);

  void connectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);
  void disconnectParam(const std::shared_ptr<AudioParam> &param, unsigned int outputIndex);

  // created for compability with the single input/ouput nodes
  virtual std::shared_ptr<AudioBus> processNode(const std::shared_ptr<AudioBus> &processingBus, int framesToProcess);
  virtual void processNode(const std::vector<std::shared_ptr<AudioBus>> &inputBuses, int framesToProcess);

  // called by an upstream node when it becomes enabled. Increments the active input counter.
  virtual void onInputEnabled();
  // called by an upstream node when it becomes disabled. Decrements the active input counter.
  virtual void onInputDisabled();

  std::vector<std::shared_ptr<AudioBus>> outputBuses_;

  BaseAudioContext *context_;
  std::unique_ptr<NodeConnections> connections_;
  std::size_t lastRenderedFrame_{SIZE_MAX};

  int numberOfInputs_ = 1;
  int numberOfOutputs_ = 1;
  int channelCount_ = 2;
  int numberOfEnabledInputNodes_ = 0;
  ChannelCountMode channelCountMode_ = ChannelCountMode::MAX;
  ChannelInterpretation channelInterpretation_ = ChannelInterpretation::SPEAKERS;


  bool isInitialized_ = false;
  bool isEnabled_ = true;

 private:
  bool isAlreadyProcessed();
  static std::string toString(ChannelCountMode mode);
  static std::string toString(ChannelInterpretation interpretation);
  void cleanup();
};

} // namespace audioapi
