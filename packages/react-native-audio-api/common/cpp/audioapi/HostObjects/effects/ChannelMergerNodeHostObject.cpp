#include <audioapi/HostObjects/effects/ChannelMergerNodeHostObject.h>

#include <audioapi/HostObjects/AudioParamHostObject.h>
#include <audioapi/core/effects/ChannelMergerNode.h>

namespace audioapi {

ChannelMergerNodeHostObject::ChannelMergerNodeHostObject(
    const std::shared_ptr<ChannelMergerNode> &node)
    : AudioNodeHostObject(node) {}

} // namespace audioapi