#pragma once

#include<memory>

#include <audioapi/core/types/ChannelCountMode.h>
#include <audioapi/core/types/ChannelInterpretation.h>
#include <audioapi/core/sources/AudioBuffer.h>

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

struct StereoPannerOptions : AudioNodeOptions {
  float pan;
  explicit StereoPannerOptions(AudioNodeOptions nodeOptions)
      : AudioNodeOptions(nodeOptions) {}
};

struct ConvolverOptions : AudioNodeOptions {
  std::shared_ptr<AudioBuffer> bus;
  bool disableNormalization;
  explicit ConvolverOptions(AudioNodeOptions nodeOptions)
      : AudioNodeOptions(nodeOptions) {}
};
} // namespace audioapi
