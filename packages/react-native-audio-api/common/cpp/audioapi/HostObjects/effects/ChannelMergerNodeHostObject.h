#pragma once

#include <audioapi/HostObjects/AudioNodeHostObject.h>

#include <memory>
#include <vector>

namespace audioapi {
using namespace facebook;

class ChannelMergerNode;

class ChannelMergerNodeHostObject : public AudioNodeHostObject {
 public:
  explicit ChannelMergerNodeHostObject(const std::shared_ptr<ChannelMergerNode> &node);
};
} // namespace audioapi
