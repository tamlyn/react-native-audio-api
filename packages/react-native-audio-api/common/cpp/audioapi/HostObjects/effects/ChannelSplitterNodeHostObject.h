#pragma once

#include <audioapi/HostObjects/AudioNodeHostObject.h>

#include <memory>
#include <vector>

namespace audioapi {
using namespace facebook;

class ChannelSplitterNode;

class ChannelSplitterNodeHostObject : public AudioNodeHostObject {
 public:
  explicit ChannelSplitterNodeHostObject(const std::shared_ptr<ChannelSplitterNode> &node);

  JSI_PROPERTY_GETTER_DECL(numberOfOutputs);
};
} // namespace audioapi
