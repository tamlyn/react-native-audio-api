#pragma once

#include <audioapi/utils/AudioArray.h>
#include <memory>
#include <vector>

namespace audioapi {
constexpr int KERNEL_SIZE = 64;
constexpr int MAX_BLOCK_SIZE = 1024;

class UpSampler {
 public:
  UpSampler();

  // N -> 2N
  void process(const std::shared_ptr<AudioArray> &input, const std::shared_ptr<AudioArray> &output);

  void reset();

 private:
  void initializeKernel();

  std::shared_ptr<AudioArray> kernel_;
  std::shared_ptr<AudioArray> stateBuffer_;
};

class DownSampler {
 public:
  DownSampler();

  // N -> N / 2
  void process(const std::shared_ptr<AudioArray> &input, const std::shared_ptr<AudioArray> &output);

  void reset();

 private:
  void initializeKernel();

  std::shared_ptr<AudioArray> kernel_;
  std::shared_ptr<AudioArray> stateBuffer_;
};

} // namespace audioapi
