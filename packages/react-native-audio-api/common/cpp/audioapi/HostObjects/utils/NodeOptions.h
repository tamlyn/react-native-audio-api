#pragma once

#include <audioapi/core/types/ChannelCountMode.h>
#include <audioapi/core/types/ChannelInterpretation.h>

namespace audioapi {
struct AudioNodeOptions {
  int channelCount;
  ChannelCountMode channelCountMode;
  ChannelInterpretation channelInterpretation;
};

struct GainOptions : AudioNodeOptions {
  float gain;
  explicit GainOptions(AudioNodeOptions nodeOptions)
      : AudioNodeOptions(nodeOptions) {}
};
} // namespace audioapi
