#pragma once

#include <audioapi/core/AudioNode.h>
#include <vector>

namespace audioapi {

class ChannelMergerNode : public AudioNode {
 public:
  explicit ChannelMergerNode(BaseAudioContext *context, unsigned int numberOfInputs = 6);
  virtual ~ChannelMergerNode() = default;

 protected:
  void processNode(const std::vector<std::shared_ptr<AudioBus>> &inputBuses, int framesToProcess) override;
};

} // namespace audioapi
