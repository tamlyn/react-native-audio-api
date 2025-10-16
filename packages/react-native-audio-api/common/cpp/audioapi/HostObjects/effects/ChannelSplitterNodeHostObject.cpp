#include <audioapi/HostObjects/effects/ChannelSplitterNodeHostObject.h>

#include <audioapi/HostObjects/AudioParamHostObject.h>
#include <audioapi/core/effects/ChannelSplitterNode.h>

namespace audioapi {

ChannelSplitterNodeHostObject::ChannelSplitterNodeHostObject(
    const std::shared_ptr<ChannelSplitterNode> &node)
    : AudioNodeHostObject(node) {
  addGetters(JSI_EXPORT_PROPERTY_GETTER(
      ChannelSplitterNodeHostObject, numberOfOutputs));
}

JSI_PROPERTY_GETTER_IMPL(ChannelSplitterNodeHostObject, numberOfOutputs) {
  auto channelSplitterNode =
      std::static_pointer_cast<ChannelSplitterNode>(node_);
  return jsi::Value(channelSplitterNode->getNumberOfOutputs());
}

} // namespace audioapi
