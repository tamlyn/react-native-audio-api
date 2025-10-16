#include <audioapi/HostObjects/effects/ChannelMergerNodeHostObject.h>

#include <audioapi/HostObjects/AudioParamHostObject.h>
#include <audioapi/core/effects/ChannelMergerNode.h>

namespace audioapi {

ChannelMergerNodeHostObject::ChannelMergerNodeHostObject(
    const std::shared_ptr<ChannelMergerNode> &node)
    : AudioNodeHostObject(node) {
  addGetters(
      JSI_EXPORT_PROPERTY_GETTER(ChannelMergerNodeHostObject, numberOfInputs));
}

JSI_PROPERTY_GETTER_IMPL(ChannelMergerNodeHostObject, numberOfInputs) {
  auto channelMergerNode = std::static_pointer_cast<ChannelMergerNode>(node_);
  return jsi::Value(channelMergerNode->getNumberOfInputs());
}

} // namespace audioapi