#pragma once

#include <memory>

#include <audioapi/core/sources/AudioBuffer.h>
#include <audioapi/core/types/BiquadFilterType.h>
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

struct AnalyserOptions : AudioNodeOptions {
  int fftSize = 2048;
  float minDecibels = -100.0f;
  float maxDecibels = -30.0f;
  float smoothingTimeConstant = 0.8f;
};

struct BiquadFilterOptions : AudioNodeOptions {
  BiquadFilterType type =
      BiquadFilterType::LOWPASS; // Uncomment and define BiquadFilterType enum as needed
  float frequency = 350.0f;
  float detune = 0.0f;
  float Q = 1.0f;
  float gain = 0.0f;
};
} // namespace audioapi
