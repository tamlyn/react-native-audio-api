#pragma once

#include <audioapi/jsi/RuntimeLifecycleMonitor.h>
#include <jsi/jsi.h>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <audioapi/HostObjects/effects/PeriodicWaveHostObject.h>
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

std::shared_ptr<BiquadFilterOptions> parseBiquadFilterOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  BiquadFilterOptions options(*nodeOptions.get());

  auto typeStr = optionsObject.getProperty(runtime, "type").asString(runtime).utf8(runtime);

  if (typeStr == "lowpass") {
    options.type = BiquadFilterType::LOWPASS;
  } else if (typeStr == "highpass") {
    options.type = BiquadFilterType::HIGHPASS;
  } else if (typeStr == "bandpass") {
    options.type = BiquadFilterType::BANDPASS;
  } else if (typeStr == "lowshelf") {
    options.type = BiquadFilterType::LOWSHELF;
  } else if (typeStr == "highshelf") {
    options.type = BiquadFilterType::HIGHSHELF;
  } else if (typeStr == "peaking") {
    options.type = BiquadFilterType::PEAKING;
  } else if (typeStr == "notch") {
    options.type = BiquadFilterType::NOTCH;
  } else if (typeStr == "allpass") {
    options.type = BiquadFilterType::ALLPASS;
  }

  options.frequency =
      static_cast<float>(optionsObject.getProperty(runtime, "frequency").getNumber());
  options.detune = static_cast<float>(optionsObject.getProperty(runtime, "detune").getNumber());
  options.Q = static_cast<float>(optionsObject.getProperty(runtime, "Q").getNumber());
  options.gain = static_cast<float>(optionsObject.getProperty(runtime, "gain").getNumber());

  return std::make_shared<BiquadFilterOptions>(options);
}

std::shared_ptr<OscillatorOptions> parseOscillatorOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  OscillatorOptions options;

  auto typeStr = optionsObject.getProperty(runtime, "type").asString(runtime).utf8(runtime);

  if (typeStr == "sine") {
    options.type = OscillatorType::SINE;
  } else if (typeStr == "square") {
    options.type = OscillatorType::SQUARE;
  } else if (typeStr == "sawtooth") {
    options.type = OscillatorType::SAWTOOTH;
  } else if (typeStr == "triangle") {
    options.type = OscillatorType::TRIANGLE;
  } else if (typeStr == "custom") {
    options.type = OscillatorType::CUSTOM;
  }

  options.frequency =
      static_cast<float>(optionsObject.getProperty(runtime, "frequency").getNumber());
  options.detune = static_cast<float>(optionsObject.getProperty(runtime, "detune").getNumber());

  if (optionsObject.hasProperty(runtime, "periodicWave")) {
    auto periodicWaveHostObject = optionsObject.getProperty(runtime, "periodicWave")
                                      .getObject(runtime)
                                      .asHostObject<PeriodicWaveHostObject>(runtime);
    options.periodicWave = periodicWaveHostObject->periodicWave_;
  }

  return std::make_shared<OscillatorOptions>(options);
}

std::shared_ptr<BaseAudioBufferSourceOptions> parseBaseAudioBufferSourceOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  BaseAudioBufferSourceOptions options;
  options.detune = static_cast<float>(optionsObject.getProperty(runtime, "detune").getNumber());
  options.playbackRate =
      static_cast<float>(optionsObject.getProperty(runtime, "playbackRate").getNumber());
  options.pitchCorrection =
      static_cast<bool>(optionsObject.getProperty(runtime, "pitchCorrection").getNumber());
  return std::make_shared<BaseAudioBufferSourceOptions>(options);
}

std::shared_ptr<AudioBufferSourceOptions> parseAudioBufferSourceOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<BaseAudioBufferSourceOptions> baseOptions =
      parseBaseAudioBufferSourceOptions(runtime, optionsObject);
  AudioBufferSourceOptions options(*baseOptions.get());
  if (optionsObject.hasProperty(runtime, "buffer")) {
    auto bufferHostObject = optionsObject.getProperty(runtime, "buffer")
                                .getObject(runtime)
                                .asHostObject<AudioBufferHostObject>(runtime);
    options.buffer = bufferHostObject->audioBuffer_;
  }
  options.loop = static_cast<bool>(optionsObject.getProperty(runtime, "loop").getNumber());
  options.loopStart =
      static_cast<float>(optionsObject.getProperty(runtime, "loopStart").getNumber());
  options.loopEnd = static_cast<float>(optionsObject.getProperty(runtime, "loopEnd").getNumber());
  return std::make_shared<AudioBufferSourceOptions>(options);
}

std::shared_ptr<StreamerOptions> parseStreamerOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  auto options = StreamerOptions();
  if (optionsObject.hasProperty(runtime, "streamPath")) {
    options.streamPath =
        optionsObject.getProperty(runtime, "streamPath").asString(runtime).utf8(runtime);
  }
  return std::make_shared<StreamerOptions>(options);
}

std::shared_ptr<AudioBufferOptions> parseAudioBufferOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  AudioBufferOptions options;
  options.numberOfChannels =
      static_cast<int>(optionsObject.getProperty(runtime, "numberOfChannels").getNumber());
  options.length = static_cast<size_t>(optionsObject.getProperty(runtime, "length").getNumber());
  options.sampleRate =
      static_cast<float>(optionsObject.getProperty(runtime, "sampleRate").getNumber());
  return std::make_shared<AudioBufferOptions>(options);
}

std::shared_ptr<DelayOptions> parseDelayOptions(
    jsi::Runtime &runtime,
    const jsi::Object &optionsObject) {
  std::shared_ptr<AudioNodeOptions> nodeOptions = parseAudioNodeOptions(runtime, optionsObject);
  DelayOptions options(*nodeOptions.get());
  options.maxDelayTime =
      static_cast<float>(optionsObject.getProperty(runtime, "maxDelayTime").getNumber());
  options.delayTime =
      static_cast<float>(optionsObject.getProperty(runtime, "delayTime").getNumber());
  return std::make_shared<DelayOptions>(options);
}
} // namespace audioapi::option_parser
