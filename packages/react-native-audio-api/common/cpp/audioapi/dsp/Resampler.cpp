#include <audioapi/dsp/Resampler.h>
#include <audioapi/utils/AudioArray.h>

namespace audioapi {
void Resampler::process(
    const std::shared_ptr<AudioArray> &input,
    const std::shared_ptr<AudioArray> &output) const {
  auto const fs = static_cast<float>(input->getSize()) /
      static_cast<float>(output->getSize());
  auto inputData = input->getData();
  auto outputData = output->getData();

  for (int i = 0; i < output->getSize(); i++) {
    auto x = (static_cast<float>(i) + 0.5f) * fs - 0.5f;
    auto minK = static_cast<int>(std::floor(x) - a_ + 1.0f);
    auto maxK = static_cast<int>(std::floor(x) + a_);

    float sum = 0.0f;
    float totalWeight = 0.0f;

    for (int k = minK; k <= maxK; k++) {
      auto clampedK = std::clamp(k, 0, static_cast<int>(input->getSize() - 1));

      auto weight = kernel(x - static_cast<float>(k));

      sum += inputData[clampedK] * weight;
      totalWeight += weight;
    }

    if (totalWeight > 0.0f) {
      outputData[i] = sum / totalWeight;
    } else [[unlikely]] {
      int nearestK = std::clamp(
          static_cast<int>(x), 0, static_cast<int>(input->getSize() - 1));
      outputData[i] = inputData[nearestK];
    }
  }
}

} // namespace audioapi
