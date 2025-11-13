#include <audioapi/core/utils/worklets/SafeIncludes.h>
#include <audioapi/dsp/Resampler.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <gtest/gtest.h>
#include <test/src/MockAudioEventHandlerRegistry.h>

using namespace audioapi;

class ResamplerTest : public ::testing::Test {
 protected:
  std::shared_ptr<Resampler> resampler_;

  void SetUp() override {
    resampler_ = std::make_shared<Resampler>()
  }
};

// number of output frames test

// quality test??? sin ???
