#pragma once

#include <audioapi/jsi/RuntimeLifecycleMonitor.h>
#include <jsi/jsi.h>
#include <memory>
#include <utility>
#include <vector>
#include <cstddef>

#include <audioapi/HostObjects/utils/NodeOptions.h>

namespace audioapi::option_parser {
  std::shared_ptr<AudioNodeOptions> parseAudioNodeOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject
  ) {
    AudioNodeOptions options;

    options.channelCount =
      static_cast<int>(optionsObject
                              .getProperty(runtime, "channelCount")
                              .getNumber());

    auto channelCountModeStr = optionsObject
                                   .getProperty(runtime, "channelCountMode")
                                   .asString(runtime)
                                   .utf8(runtime);

    if (channelCountModeStr == "max") {
      options.channelCountMode = ChannelCountMode::MAX;
    } else if (channelCountModeStr == "clamped-max") {
      options.channelCountMode = ChannelCountMode::CLAMPED_MAX;
    } else if (channelCountModeStr == "explicit") {
      options.channelCountMode = ChannelCountMode::EXPLICIT;
    }

    auto channelInterpretationStr = optionsObject
                                        .getProperty(runtime, "channelInterpretation")
                                        .asString(runtime)
                                        .utf8(runtime);

    if (channelInterpretationStr == "speakers") {
      options.channelInterpretation = ChannelInterpretation::SPEAKERS;
    } else if (channelInterpretationStr == "discrete") {
      options.channelInterpretation = ChannelInterpretation::DISCRETE;
    }

    return std::make_shared<AudioNodeOptions>(options);
  }

  GainOptions parseGainOptions(
      jsi::Runtime &runtime,
      const jsi::Object &optionsObject) {
    std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
    GainOptions options(*nodeOptions.get());
    options.gain = static_cast<float>(optionsObject
                                          .getProperty(runtime, "gain")
                                          .getNumber());
    return options;
  }
} // namespace audioapi::option_parser


