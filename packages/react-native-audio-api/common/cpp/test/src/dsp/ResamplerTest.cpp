#include <audioapi/core/utils/worklets/SafeIncludes.h>
#include <audioapi/dsp/Resampler.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/core/utils/Constants.h>
#include <gtest/gtest.h>
#include <memory>

using namespace audioapi;

class ResamplerTest : public ::testing::Test {
  protected:
    static constexpr int KERNEL_SIZE = RENDER_QUANTUM_SIZE;
};

class TestableUpSampler : public UpSampler {
 public:
  explicit TestableUpSampler(int maxBlockSize, int kernelSize)
      : UpSampler(maxBlockSize, kernelSize) {}

  std::shared_ptr<AudioArray> getKernel() {
    return kernel_;
  }
};

class TestableDownSampler : public DownSampler {
 public:
  explicit TestableDownSampler(int maxBlockSize, int kernelSize)
      : DownSampler(maxBlockSize, kernelSize) {}

  std::shared_ptr<AudioArray> getKernel() {
    return kernel_;
  }
};

TEST_F(ResamplerTest, UpSamplerCanBeCreated) {
  auto upSampler = std::make_unique<UpSampler>(RENDER_QUANTUM_SIZE, RENDER_QUANTUM_SIZE);
  ASSERT_NE(upSampler, nullptr);
}

TEST_F(ResamplerTest, DownSamplerCanBeCreated) {
  auto downSampler = std::make_unique<DownSampler>(RENDER_QUANTUM_SIZE * 2, RENDER_QUANTUM_SIZE * 2);
  ASSERT_NE(downSampler, nullptr);
}

TEST_F(ResamplerTest, UpSamplerKernelSymmetry) {
  auto upSampler = std::make_unique<TestableUpSampler>(RENDER_QUANTUM_SIZE, RENDER_QUANTUM_SIZE);
  auto kernel = upSampler->getKernel();

  // check for symmetry around the center point
  for (size_t i = 0; i < kernel->getSize() / 2; ++i) {
    EXPECT_NEAR((*kernel)[i], (*kernel)[kernel->getSize() - 1 - i], 1e-6);
  }
}

TEST_F(ResamplerTest, DownSamplerKernelSymmetry) {
  auto downSampler =
      std::make_unique<TestableDownSampler>(RENDER_QUANTUM_SIZE * 2, RENDER_QUANTUM_SIZE * 2);
  auto kernel = downSampler->getKernel();

  // check for symmetry around the center point
  // as the kernel size is even, we compare pairs around the center -> kernel[size/2 - 1]
  // last value is skipped
  for (size_t i = 0; i < kernel->getSize() / 2; ++i) {
    EXPECT_NEAR((*kernel)[i], (*kernel)[kernel->getSize() - 2 - i], 1e-6);
  }

  EXPECT_FLOAT_EQ((*kernel)[kernel->getSize() / 2 - 1], 0.5f);
}

TEST_F(ResamplerTest, UpSamplerKernelSum) {
  auto upSampler = std::make_unique<TestableUpSampler>(RENDER_QUANTUM_SIZE, RENDER_QUANTUM_SIZE);
  auto kernel = upSampler->getKernel();

  float sum = 0.0f;
  for (size_t i = 0; i < kernel->getSize(); ++i) {
    sum += (*kernel)[i];
  }

  EXPECT_NEAR(sum, 1.0f, 1e-6);
}

TEST_F(ResamplerTest, DownSamplerKernelSum) {
  auto downSampler =
      std::make_unique<TestableDownSampler>(RENDER_QUANTUM_SIZE * 2, RENDER_QUANTUM_SIZE * 2);
  auto kernel = downSampler->getKernel();

  float sum = 0.0f;
  for (size_t i = 0; i < kernel->getSize(); ++i) {
    sum += (*kernel)[i];
  }

  EXPECT_NEAR(sum, 1.0f, 1e-6);
}

TEST_F(ResamplerTest, UpSamplerProcess) {
  auto upSampler = std::make_unique<TestableUpSampler>(RENDER_QUANTUM_SIZE, RENDER_QUANTUM_SIZE);
  auto kernel = upSampler->getKernel();

  auto stateBuffer = std::make_shared<AudioArray>(2 * RENDER_QUANTUM_SIZE);
  stateBuffer->zero();

  auto inputArray = std::make_shared<AudioArray>(4);
  (*inputArray)[0] = 1.0f;
  (*inputArray)[1] = 0.0f;
  (*inputArray)[2] = -1.0f;
  (*inputArray)[3] = 1.0f;

  auto outputArray = std::make_shared<AudioArray>(8);

  memcpy(
      stateBuffer->getData() + KERNEL_SIZE,
      inputArray->getData(),
      inputArray->getSize() * sizeof(float));

  upSampler->process(inputArray, outputArray, 4);

  for (size_t i = 0; i < outputArray->getSize(); ++i) {
    auto idx = RENDER_QUANTUM_SIZE - RENDER_QUANTUM_SIZE / 2 + i / 2;

    if (i % 2 == 0) {
      EXPECT_FLOAT_EQ(outputArray->getData()[i], stateBuffer->getData()[idx]);
    } else {
      float sum = 0.0f;

      for (size_t k = 0; k < kernel->getSize(); ++k) {
        sum += stateBuffer->getData()[i / 2 + 1 + k] * kernel->getData()[k];
      }

      EXPECT_FLOAT_EQ(outputArray->getData()[i], sum);
    }
  }
}

TEST_F(ResamplerTest, DownSamplerProcess) {
  auto downSampler =
      std::make_unique<TestableDownSampler>(RENDER_QUANTUM_SIZE * 2, RENDER_QUANTUM_SIZE * 2);
  auto kernel = downSampler->getKernel();

  auto stateBuffer = std::make_shared<AudioArray>(4 * RENDER_QUANTUM_SIZE);
  stateBuffer->zero();

  auto inputArray = std::make_shared<AudioArray>(8);
  (*inputArray)[0] = 1.0f;
  (*inputArray)[1] = 0.0f;
  (*inputArray)[2] = -1.0f;
  (*inputArray)[3] = 1.0f;
  (*inputArray)[4] = 0.5f;
  (*inputArray)[5] = -0.5f;
  (*inputArray)[6] = 0.25f;
  (*inputArray)[7] = -0.25f;

  auto outputArray = std::make_shared<AudioArray>(4);

  memcpy(
      stateBuffer->getData() + KERNEL_SIZE,
      inputArray->getData(),
      inputArray->getSize() * sizeof(float));

  downSampler->process(inputArray, outputArray, 8);

  for (size_t i = 0; i < outputArray->getSize() / 8; ++i) {
    auto idx = RENDER_QUANTUM_SIZE + i * 2;
    float sum = 0.0f;

    for (size_t k = 0; k < kernel->getSize(); ++k) {
      sum += stateBuffer->getData()[idx + k] * kernel->getData()[k];
    }

    EXPECT_FLOAT_EQ(outputArray->getData()[i], sum);
  }
}

TEST_F(ResamplerTest, UpDownSamplingProcessThrowsNoErrors) {
  auto upSampler = std::make_unique<TestableUpSampler>(RENDER_QUANTUM_SIZE, RENDER_QUANTUM_SIZE);
  auto downSampler =
      std::make_unique<TestableDownSampler>(RENDER_QUANTUM_SIZE * 2, RENDER_QUANTUM_SIZE * 2);

  auto inputArray = std::make_shared<AudioArray>(4);
  (*inputArray)[0] = 1.0f;
  (*inputArray)[1] = 0.0f;
  (*inputArray)[2] = -1.0f;
  (*inputArray)[3] = 1.0f;

  auto outputArray = std::make_shared<AudioArray>(8);

  EXPECT_NO_THROW(upSampler->process(inputArray, outputArray, 4));
  EXPECT_NO_THROW(downSampler->process(outputArray, inputArray, 8));
}
