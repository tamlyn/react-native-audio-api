#pragma once

#include <audioapi/core/AudioNode.h>
#include <vector>

namespace audioapi {

class ChannelSplitterNode : public AudioNode {
 public:
  explicit ChannelSplitterNode(BaseAudioContext *context, unsigned numberOfOutputs = 6);
  virtual ~ChannelSplitterNode() = default;

 protected:
  void processNode(const std::vector<std::shared_ptr<AudioBus>> &inputBuses, int framesToProcess) override;
};

} // namespace audioapi
