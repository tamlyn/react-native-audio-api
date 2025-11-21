#include <audioapi/core/OfflineAudioContext.h>
#include <audioapi/core/effects/WaveShaperNode.h>
#include <audioapi/core/utils/worklets/SafeIncludes.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <gtest/gtest.h>
#include <test/src/MockAudioEventHandlerRegistry.h>
#include <memory>

using namespace audioapi;

class WaveShaperNodeTest : public ::testing::Test {
 protected:
  std::shared_ptr<MockAudioEventHandlerRegistry> eventRegistry;
  std::unique_ptr<OfflineAudioContext> context;
  static constexpr int sampleRate = 44100;

  void SetUp() override {
    eventRegistry = std::make_shared<MockAudioEventHandlerRegistry>();
    context = std::make_unique<OfflineAudioContext>(
        2, 5 * sampleRate, sampleRate, eventRegistry, RuntimeRegistry{});
  }
};

class TestableWaveShaperNode : public WaveShaperNode {
 public:
  explicit TestableStereoPannerNode(BaseAudioContext *context) : StereoPannerNode(context) {}

  std::shared_ptr<AudioBus> processNode(
      const std::shared_ptr<AudioBus> &processingBus,
      int framesToProcess) override {
    return WaveShaperNode::processNode(processingBus, framesToProcess);
  }
};

TEST_F(WaveShaperNodeTest, WaveShaperNodeCanBeCreated) {
  auto waveShaper = context->createWaveShaper();
  ASSERT_NE(waveShaper, nullptr);
}

// curve alg test1, test2...

// no curve test

// does none, 2x, 4x return correct number of frames
