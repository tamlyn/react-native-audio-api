#include <audioapi/HostObjects/effects/ChannelSplitterNodeHostObject.h>

#include <audioapi/HostObjects/AudioParamHostObject.h>
#include <audioapi/core/effects/ChannelSplitterNode.h>

namespace audioapi {

ChannelSplitterNodeHostObject::ChannelSplitterNodeHostObject(
    const std::shared_ptr<ChannelSplitterNode> &node)
    : AudioNodeHostObject(node) {}

} // namespace audioapi
