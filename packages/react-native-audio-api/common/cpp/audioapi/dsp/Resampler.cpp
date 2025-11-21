#include <audioapi/dsp/Resampler.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers> // Use <math.h> and M_PI if not using C++20

namespace audioapi {

static constexpr float PI = std::numbers::pi_v<float>;

static float blackmanWindow(int index) {
  double alpha = 0.16;
  double a0 = 0.5 * (1.0 - alpha);
  double a1 = 0.5;
  double a2 = 0.5 * alpha;
  double n = static_cast<double>(index) / KERNEL_SIZE;
  return static_cast<float>(
      a0 - a1 * std::cos(2.0 * PI * n) + a2 * std::cos(4.0 * PI * n));
}

UpSampler::UpSampler() {
  kernel_ = std::make_shared<AudioArray>(KERNEL_SIZE);
  stateBuffer_ = std::make_shared<AudioArray>(KERNEL_SIZE + MAX_BLOCK_SIZE);
  stateBuffer_->zero();
  initializeKernel();
}

void UpSampler::initializeKernel() {
  float *kData = kernel_->getData();
  int halfSize = KERNEL_SIZE / 2;

  for (int i = 0; i < KERNEL_SIZE; ++i) {
    auto x = static_cast<double>(i - halfSize);
    double sinc =
        (std::abs(x) < 1e-9) ? 1.0 : std::sin(x * PI * 0.5) / (x * PI * 0.5);
    kData[i] = static_cast<float>(sinc * blackmanWindow(i));
  }
}

void UpSampler::reset() {
  if (stateBuffer_)
    stateBuffer_->zero();
}

void UpSampler::process(
    const std::shared_ptr<AudioArray> &input,
    const std::shared_ptr<AudioArray> &output) {

  auto inputSize = input->getSize();
  if (inputSize > MAX_BLOCK_SIZE)
    return;
  if (output->getSize() < inputSize * 2)
    return;

  const float *inputData = input->getData();
  float *outputData = output->getData();
  float *state = stateBuffer_->getData();
  const float *kernel = kernel_->getData();

  // move previous tail to front
  std::memmove(state, state + inputSize, KERNEL_SIZE * sizeof(float));
  // copy new input
  std::memcpy(state + KERNEL_SIZE, inputData, inputSize * sizeof(float));

  int halfKernel = KERNEL_SIZE / 2;

  for (int i = 0; i < inputSize; ++i) {
    int centerIdx = KERNEL_SIZE + i;

    outputData[2 * i] = state[centerIdx];

    float sum = 0.0f;
    for (int k = 0; k < KERNEL_SIZE; ++k) {
      sum += state[centerIdx - halfKernel + k] * kernel[KERNEL_SIZE - 1 - k];
    }
    outputData[2 * i + 1] = sum;
  }
}

DownSampler::DownSampler() {
  kernel_ = std::make_shared<AudioArray>(KERNEL_SIZE);
  stateBuffer_ =
      std::make_shared<AudioArray>(KERNEL_SIZE + (MAX_BLOCK_SIZE * 2));
  stateBuffer_->zero();
  initializeKernel();
}

void DownSampler::initializeKernel() {
  float *kData = kernel_->getData();
  int halfSize = KERNEL_SIZE / 2;

  for (int i = 0; i < KERNEL_SIZE; ++i) {
    auto x = static_cast<double>(i - halfSize);
    double sinc =
        (std::abs(x) < 1e-9) ? 1.0 : std::sin(x * PI * 0.5) / (x * PI * 0.5);
    kData[i] = static_cast<float>(sinc * blackmanWindow(i));
  }
}

void DownSampler::reset() {
  if (stateBuffer_)
    stateBuffer_->zero();
}

void DownSampler::process(
    const std::shared_ptr<AudioArray> &input,
    const std::shared_ptr<AudioArray> &output) {
  auto inputSize = input->getSize();
  if (inputSize > MAX_BLOCK_SIZE * 2)
    return;
  if (output->getSize() < inputSize / 2)
    return;

  const float *inputData = input->getData();
  float *outputData = output->getData();
  float *state = stateBuffer_->getData();
  const float *kernel = kernel_->getData();
  int halfKernel = KERNEL_SIZE / 2;

  // move history (shift by amount processed last time, which is inputSize)
  std::memmove(state, state + inputSize, KERNEL_SIZE * sizeof(float));
  // copy new input
  std::memcpy(state + KERNEL_SIZE, inputData, inputSize * sizeof(float));

  auto outputCount = inputSize / 2;

  for (int i = 0; i < outputCount; ++i) {
    int centerIdx = KERNEL_SIZE + (2 * i);

    float sum = 0.0f;
    for (int k = 0; k < KERNEL_SIZE; ++k) {
      sum += state[centerIdx - halfKernel + k] * kernel[KERNEL_SIZE - 1 - k];
    }
    outputData[i] = sum;
  }
}

} // namespace audioapi
