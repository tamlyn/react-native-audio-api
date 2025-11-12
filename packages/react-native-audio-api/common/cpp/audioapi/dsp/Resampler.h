#pragma once

#include <memory>

namespace audioapi {

class AudioArray;

class Resampler {

public:
    explicit Resampler(float a = 3.0f) : a_(a) {}

    // https://en.wikipedia.org/wiki/Lanczos_resampling
    void process(const std::shared_ptr<AudioArray> &input, const std::shared_ptr<AudioArray> &output) const;

private:
  float a_;

  // Lanczos kernel function
  [[nodiscard]] inline float kernel(float x) const {
      if (std::abs(x) < 1e-6) {
        return 1.0f;
      } else if (std::abs(x) >= a_) {
        return 0.0f;
      } else {
        auto piX = static_cast<float>(M_PI * x);
        return (a_ * std::sin(piX) * std::sin(piX / a_)) / (piX * piX);
      }
  }
};
} // namespace audioapi
