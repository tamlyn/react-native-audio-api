#pragma once

#include <audioapi/utils/AudioArray.h>
#include <memory>
#include <vector>

namespace audioapi {
constexpr int KERNEL_SIZE = 128;
constexpr int MAX_BLOCK_SIZE = 1024;

class Resampler {
 public:
  Resampler();
  virtual ~Resampler() = default;

  virtual int process(
      const std::shared_ptr<AudioArray> &input,
      const std::shared_ptr<AudioArray> &output,
      int framesToProcess) = 0;
  void reset();

 protected:
  static float computeConvolution(const float *stateStart, const float *kernelStart);
  virtual void initializeKernel() = 0;

  std::shared_ptr<AudioArray> kernel_;
  std::shared_ptr<AudioArray> stateBuffer_;
};

class UpSampler : public Resampler {
 public:
  UpSampler();

  // N -> 2N
  int process(const std::shared_ptr<AudioArray> &input, const std::shared_ptr<AudioArray> &output, int framesToProcess)
      override;

 protected:
  void initializeKernel() final;
};

class DownSampler : public Resampler {
 public:
  DownSampler();

  // N -> N / 2
  int process(const std::shared_ptr<AudioArray> &input, const std::shared_ptr<AudioArray> &output, int framesToProcess)
      override;

 protected:
  void initializeKernel() final;
};

} // namespace audioapi
