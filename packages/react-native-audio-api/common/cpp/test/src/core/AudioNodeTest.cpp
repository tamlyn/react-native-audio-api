#include <audioapi/core/OfflineAudioContext.h>
#include <audioapi/core/destinations/AudioDestinationNode.h>
#include <audioapi/core/effects/GainNode.h>
#include <audioapi/core/sources/ConstantSourceNode.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/core/utils/worklets/SafeIncludes.h>
#include <audioapi/types/NodeOptions.h>
#include <audioapi/utils/AudioArray.hpp>
#include <audioapi/utils/AudioBuffer.hpp>
#include <gtest/gtest.h>
#include <test/src/MockAudioEventHandlerRegistry.h>
#include <memory>

using namespace audioapi;

// NOLINTBEGIN

class AudioNodeBranchingTest : public ::testing::Test {
 protected:
  std::shared_ptr<MockAudioEventHandlerRegistry> eventRegistry;
  std::shared_ptr<OfflineAudioContext> context;
  static constexpr int sampleRate = 44100;

  void SetUp() override {
    eventRegistry = std::make_shared<MockAudioEventHandlerRegistry>();
    context = std::make_shared<OfflineAudioContext>(
        2, 5 * sampleRate, sampleRate, eventRegistry, RuntimeRegistry{});
    context->initialize();
  }

  // Run one render quantum synchronously and return the destination buffer.
  std::shared_ptr<DSPAudioBuffer> renderQuantum() {
    auto out = std::make_shared<DSPAudioBuffer>(RENDER_QUANTUM_SIZE, 2, sampleRate);
    context->getDestination()->renderAudio(out, RENDER_QUANTUM_SIZE);
    return out;
  }
};

// Regression test for issue #933 — branching signal paths.
//
// Topology:
//   ConstantSource(1.0) ──► dryGain(1.0) ──► destination
//                       └─► wetGain(0.0) ──► destination
//
// Before the fix, a node's processInputs() would return the input's own
// buffer when that input had the most channels. GainNode::processNode()
// then multiplied that buffer in place, which (at gain=0) zeroed the
// shared upstream buffer that the dry path was also reading from. Result:
// silence on the destination instead of the dry signal.
TEST_F(AudioNodeBranchingTest, MutedBranchDoesNotSilenceOtherBranch) {
  auto source = context->createConstantSource(ConstantSourceOptions());
  source->getOffsetParam()->setValue(1.0f);
  source->start(0.0);

  auto dryGain = context->createGain(GainOptions());
  dryGain->getGainParam()->setValue(1.0f);

  auto wetGain = context->createGain(GainOptions());
  wetGain->getGainParam()->setValue(0.0f);

  source->connect(dryGain);
  source->connect(wetGain);
  dryGain->connect(context->getDestination());
  wetGain->connect(context->getDestination());

  auto out = renderQuantum();

  // Dry path passes 1.0; wet path is silenced. Expect ~1.0 on every frame.
  for (size_t ch = 0; ch < out->getNumberOfChannels(); ++ch) {
    for (int i = 0; i < RENDER_QUANTUM_SIZE; ++i) {
      EXPECT_NEAR((*out->getChannel(ch))[i], 1.0f, 1e-5f)
          << "channel " << ch << " frame " << i;
    }
  }
}

// Symmetric case: at the other slider extreme, only the wet path is open.
// With wet gain = 1.0 and dry gain = 0.0, output should be ~1.0 (the
// constant source passing through the wet gain unmodified).
TEST_F(AudioNodeBranchingTest, MutedDryDoesNotSilenceWet) {
  auto source = context->createConstantSource(ConstantSourceOptions());
  source->getOffsetParam()->setValue(1.0f);
  source->start(0.0);

  auto dryGain = context->createGain(GainOptions());
  dryGain->getGainParam()->setValue(0.0f);

  auto wetGain = context->createGain(GainOptions());
  wetGain->getGainParam()->setValue(1.0f);

  source->connect(dryGain);
  source->connect(wetGain);
  dryGain->connect(context->getDestination());
  wetGain->connect(context->getDestination());

  auto out = renderQuantum();

  for (size_t ch = 0; ch < out->getNumberOfChannels(); ++ch) {
    for (int i = 0; i < RENDER_QUANTUM_SIZE; ++i) {
      EXPECT_NEAR((*out->getChannel(ch))[i], 1.0f, 1e-5f)
          << "channel " << ch << " frame " << i;
    }
  }
}

// Midpoint: both branches open at 0.5. Output should be 1.0 (0.5 + 0.5).
TEST_F(AudioNodeBranchingTest, EqualMixSumsBothBranches) {
  auto source = context->createConstantSource(ConstantSourceOptions());
  source->getOffsetParam()->setValue(1.0f);
  source->start(0.0);

  auto dryGain = context->createGain(GainOptions());
  dryGain->getGainParam()->setValue(0.5f);

  auto wetGain = context->createGain(GainOptions());
  wetGain->getGainParam()->setValue(0.5f);

  source->connect(dryGain);
  source->connect(wetGain);
  dryGain->connect(context->getDestination());
  wetGain->connect(context->getDestination());

  auto out = renderQuantum();

  for (size_t ch = 0; ch < out->getNumberOfChannels(); ++ch) {
    for (int i = 0; i < RENDER_QUANTUM_SIZE; ++i) {
      EXPECT_NEAR((*out->getChannel(ch))[i], 1.0f, 1e-5f)
          << "channel " << ch << " frame " << i;
    }
  }
}

// NOLINTEND
