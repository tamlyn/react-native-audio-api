#pragma once

#include <audioapi/jsi/RuntimeLifecycleMonitor.h>
#include <jsi/jsi.h>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <audioapi/HostObjects/sources/AudioBufferHostObject.h>
#include <audioapi/HostObjects/utils/NodeOptions.h>

namespace audioapi::option_parser {
std::shared_ptr<AudioNodeOptions> parseAudioNodeOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  AudioNodeOptions options;

  options.channelCount =
      static_cast<int>(optionsObject.getProperty(runtime, "channelCount").getNumber());

  auto channelCountModeStr =
      optionsObject.getProperty(runtime, "channelCountMode").asString(runtime).utf8(runtime);

  if (channelCountModeStr == "max") {
    options.channelCountMode = ChannelCountMode::MAX;
  } else if (channelCountModeStr == "clamped-max") {
    options.channelCountMode = ChannelCountMode::CLAMPED_MAX;
  } else if (channelCountModeStr == "explicit") {
    options.channelCountMode = ChannelCountMode::EXPLICIT;
  }

  auto channelInterpretationStr =
      optionsObject.getProperty(runtime, "channelInterpretation").asString(runtime).utf8(runtime);

  if (channelInterpretationStr == "speakers") {
    options.channelInterpretation = ChannelInterpretation::SPEAKERS;
  } else if (channelInterpretationStr == "discrete") {
    options.channelInterpretation = ChannelInterpretation::DISCRETE;
  }

  return std::make_shared<AudioNodeOptions>(options);
}

std::shared_ptr<GainOptions> parseGainOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  GainOptions options(*nodeOptions.get());
  options.gain = static_cast<float>(optionsObject.getProperty(runtime, "gain").getNumber());
  return std::make_shared<GainOptions>(options);
}

std::shared_ptr<StereoPannerOptions> parseStereoPannerOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  StereoPannerOptions options(*nodeOptions.get());
  options.pan = static_cast<float>(optionsObject.getProperty(runtime, "pan").getNumber());
  return std::make_shared<StereoPannerOptions>(options);
}

std::shared_ptr<ConvolverOptions> parseConvolverOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  ConvolverOptions options(*nodeOptions.get());
  options.disableNormalization =
      static_cast<bool>(optionsObject.getProperty(runtime, "disableNormalization").getNumber());
  if (optionsObject.hasProperty(runtime, "buffer")) {
    auto bufferHostObject = optionsObject.getProperty(runtime, "buffer")
                                .getObject(runtime)
                                .asHostObject<AudioBufferHostObject>(runtime);
    options.bus = bufferHostObject->audioBuffer_;
  }
  return std::make_shared<ConvolverOptions>(options);
}

std::shared_ptr<ConstantSourceOptions> parseConstantSourceOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  ConstantSourceOptions options;
  options.offset = static_cast<float>(optionsObject.getProperty(runtime, "offset").getNumber());
  return std::make_shared<ConstantSourceOptions>(options);
}

std::shared_ptr<AnalyserOptions> parseAnalyserOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  AnalyserOptions options(*nodeOptions.get());
  options.fftSize = static_cast<int>(optionsObject.getProperty(runtime, "fftSize").getNumber());
  options.minDecibels =
      static_cast<float>(optionsObject.getProperty(runtime, "minDecibels").getNumber());
  options.maxDecibels =
      static_cast<float>(optionsObject.getProperty(runtime, "maxDecibels").getNumber());
  options.smoothingTimeConstant =
      static_cast<float>(optionsObject.getProperty(runtime, "smoothingTimeConstant").getNumber());
  return std::make_shared<AnalyserOptions>(options);
}
} // namespace audioapi::option_parser
