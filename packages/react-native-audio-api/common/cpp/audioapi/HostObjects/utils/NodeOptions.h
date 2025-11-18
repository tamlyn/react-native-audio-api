#pragma once

#include <memory>

#include <audioapi/core/sources/AudioBuffer.h>
#include <audioapi/core/types/ChannelCountMode.h>
#include <audioapi/core/types/ChannelInterpretation.h>

namespace audioapi {
struct AudioNodeOptions {
  int channelCount = 2;
  ChannelCountMode channelCountMode = ChannelCountMode::MAX;
  ChannelInterpretation channelInterpretation = ChannelInterpretation::SPEAKERS;
};

struct GainOptions : AudioNodeOptions {
  float gain = 1.0f;
};

struct StereoPannerOptions : AudioNodeOptions {
  float pan = 0.0f;
};

struct ConvolverOptions : AudioNodeOptions {
  std::shared_ptr<AudioBuffer> bus = nullptr;
  bool disableNormalization = false;
};

struct ConstantSourceOptions {
  float offset = 1.0f;
};
} // namespace audioapi
