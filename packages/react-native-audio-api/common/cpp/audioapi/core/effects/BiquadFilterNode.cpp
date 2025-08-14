#include <audioapi/core/BaseAudioContext.h>
#include <audioapi/core/effects/BiquadFilterNode.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>

// https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html - math
// formulas for filters

namespace audioapi {

BiquadFilterNode::BiquadFilterNode(BaseAudioContext *context)
    : AudioNode(context) {
  frequencyParam_ = std::make_shared<AudioParam>(
      350.0, 0.0f, context->getNyquistFrequency(), context);
  detuneParam_ = std::make_shared<AudioParam>(
      0.0,
      -1200 * LOG2_MOST_POSITIVE_SINGLE_FLOAT,
      1200 * LOG2_MOST_POSITIVE_SINGLE_FLOAT,
      context);
  QParam_ = std::make_shared<AudioParam>(
      1.0, MOST_NEGATIVE_SINGLE_FLOAT, MOST_POSITIVE_SINGLE_FLOAT, context);
  gainParam_ = std::make_shared<AudioParam>(
      0.0,
      MOST_NEGATIVE_SINGLE_FLOAT,
      40 * LOG10_MOST_POSITIVE_SINGLE_FLOAT,
      context);
  type_ = BiquadFilterType::LOWPASS;
  isInitialized_ = true;
  channelCountMode_ = ChannelCountMode::MAX;
}

std::string BiquadFilterNode::getType() {
  return BiquadFilterNode::toString(type_);
}

void BiquadFilterNode::setType(const std::string &type) {
  type_ = BiquadFilterNode::fromString(type);
}

std::shared_ptr<AudioParam> BiquadFilterNode::getFrequencyParam() const {
  return frequencyParam_;
}

std::shared_ptr<AudioParam> BiquadFilterNode::getDetuneParam() const {
  return detuneParam_;
}

std::shared_ptr<AudioParam> BiquadFilterNode::getQParam() const {
  return QParam_;
}

std::shared_ptr<AudioParam> BiquadFilterNode::getGainParam() const {
  return gainParam_;
}

// Compute Z-transform of the filter
// https://www.dsprelated.com/freebooks/filters/Frequency_Response_Analysis.html
// https://www.dsprelated.com/freebooks/filters/Transfer_Function_Analysis.html
//
// frequency response -  H(z)
//          b0 + b1 * z^(-1) + b2 * z^(-2)
//  H(z) = -------------------------------
//           1 + a1 * z^(-1) + a2 * z^(-2)
//
//         b0 + (b1 + b2 * z1) * z1
//     =  --------------------------
//         (1 + (a1 + a2 * z1) * z1
//
// where z1 = 1/z and z = e^(j * pi * frequency)
// z1 = e^(-j * pi * frequency)
//
// phase response - angle of the frequency response
//

void BiquadFilterNode::getFrequencyResponse(
    const float *frequencyArray,
    float *magResponseOutput,
    float *phaseResponseOutput,
    const int length) {
  // Local copies for micro-optimization
  float b0 = b0_[0];
  float b1 = b1_[0];
  float b2 = b2_[0];
  float a1 = a1_[0];
  float a2 = a2_[0];

  for (size_t i = 0; i < length; i++) {
    if (frequencyArray[i] < 0.0 || frequencyArray[i] > 1.0) {
      magResponseOutput[i] = std::nanf("");
      phaseResponseOutput[i] = std::nanf("");
      continue;
    }

    auto omega = -PI * frequencyArray[i];
    auto z = std::complex<float>(cos(omega), sin(omega));
    auto response = (b0 + (b1 + b2 * z) * z) /
        (std::complex<float>(1, 0) + (a1 + a2 * z) * z);
    magResponseOutput[i] = static_cast<float>(std::abs(response));
    phaseResponseOutput[i] =
        static_cast<float>(atan2(imag(response), real(response)));
  }
}

void BiquadFilterNode::resetCoefficients() {
  x1_ = 0.0;
  x2_ = 0.0;
  y1_ = 0.0;
  y2_ = 0.0;
}

void BiquadFilterNode::setNormalizedCoefficients(
    size_t index,
    float b0,
    float b1,
    float b2,
    float a0,
    float a1,
    float a2) {
  auto a0Inverted = 1.0f / a0;
  b0_[index] = b0 * a0Inverted;
  b1_[index] = b1 * a0Inverted;
  b2_[index] = b2 * a0Inverted;
  a1_[index] = a1 * a0Inverted;
  a2_[index] = a2 * a0Inverted;
}

void BiquadFilterNode::setLowpassCoefficients(
    size_t index,
    float frequency,
    float Q) {
  // Limit frequency to [0, 1] range
  if (frequency >= 1.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (frequency <= 0.0) {
    setNormalizedCoefficients(index, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  Q = std::max(0.0f, Q);
  float g = std::pow(10.0f, 0.05f * Q);

  float theta = PI * frequency;
  float alpha = std::sin(theta) / (2 * g);
  float cosW = std::cos(theta);
  float beta = (1 - cosW) / 2;

  setNormalizedCoefficients(
      index, beta, 2 * beta, beta, 1 + alpha, -2 * cosW, 1 - alpha);
}

void BiquadFilterNode::setHighpassCoefficients(
    size_t index,
    float frequency,
    float Q) {
  if (frequency >= 1.0) {
    setNormalizedCoefficients(index, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }
  if (frequency <= 0.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  Q = std::max(0.0f, Q);
  float g = std::pow(10.0f, 0.05f * Q);

  float theta = PI * frequency;
  float alpha = std::sin(theta) / (2 * g);
  float cosW = std::cos(theta);
  float beta = (1 - cosW) / 2;

  setNormalizedCoefficients(
      index, beta, -2 * beta, beta, 1 + alpha, -2 * cosW, 1 - alpha);
}

void BiquadFilterNode::setBandpassCoefficients(
    size_t index,
    float frequency,
    float Q) {
  // Limit frequency to [0, 1] range
  if (frequency <= 0.0 || frequency >= 1.0) {
    setNormalizedCoefficients(index, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  // Limit Q to positive values
  if (Q <= 0.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  float alpha = std::sin(w0) / (2 * Q);
  float cosW = std::cos(w0);

  setNormalizedCoefficients(
      index, alpha, 0.0f, -alpha, 1.0f + alpha, -2 * cosW, 1.0f - alpha);
}

void BiquadFilterNode::setLowshelfCoefficients(
    size_t index,
    float frequency,
    float gain) {
  float A = std::pow(10.0f, gain / 40.0f);

  if (frequency >= 1.0) {
    setNormalizedCoefficients(index, A * A, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (frequency <= 0.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  float alpha = 0.5f * std::sin(w0) * std::sqrt(2.0f);
  float cosW = std::cos(w0);
  float gamma = 2.0f * std::sqrt(A) * alpha;

  setNormalizedCoefficients(
      index,
      A * (A + 1 - (A - 1) * cosW + gamma),
      2.0f * A * (A - 1 - (A + 1) * cosW),
      A * (A + 1 - (A - 1) * cosW - gamma),
      A + 1 + (A - 1) * cosW + gamma,
      -2.0f * (A - 1 + (A + 1) * cosW),
      A + 1 + (A - 1) * cosW - gamma);
}

void BiquadFilterNode::setHighshelfCoefficients(
    size_t index,
    float frequency,
    float gain) {
  float A = std::pow(10.0f, gain / 40.0f);

  if (frequency >= 1.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (frequency <= 0.0) {
    setNormalizedCoefficients(index, A * A, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  // In the original formula: sqrt((A + 1/A) * (1/S - 1) + 2), but we assume
  // the maximum value S = 1, so it becomes 0 + 2 under the square root
  float alpha = 0.5f * std::sin(w0) * std::sqrt(2.0f);
  float cosW = std::cos(w0);
  float gamma = 2.0f * std::sqrt(A) * alpha;

  setNormalizedCoefficients(
      index,
      A * (A + 1 + (A - 1) * cosW + gamma),
      -2.0f * A * (A - 1 + (A + 1) * cosW),
      A * (A + 1 + (A - 1) * cosW - gamma),
      A + 1 - (A - 1) * cosW + gamma,
      2.0f * (A - 1 - (A + 1) * cosW),
      A + 1 - (A - 1) * cosW - gamma);
}

void BiquadFilterNode::setPeakingCoefficients(
    size_t index,
    float frequency,
    float Q,
    float gain) {
  float A = std::pow(10.0f, gain / 40.0f);

  if (frequency <= 0.0 || frequency >= 1.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (Q <= 0.0) {
    setNormalizedCoefficients(index, A * A, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  float alpha = std::sin(w0) / (2 * Q);
  float cosW = std::cos(w0);

  setNormalizedCoefficients(
      index,
      1 + alpha * A,
      -2 * cosW,
      1 - alpha * A,
      1 + alpha / A,
      -2 * cosW,
      1 - alpha / A);
}

void BiquadFilterNode::setNotchCoefficients(
    size_t index,
    float frequency,
    float Q) {
  if (frequency <= 0.0 || frequency >= 1.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (Q <= 0.0) {
    setNormalizedCoefficients(index, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  float alpha = std::sin(w0) / (2 * Q);
  float cosW = std::cos(w0);

  setNormalizedCoefficients(
      index, 1.0f, -2 * cosW, 1.0f, 1 + alpha, -2 * cosW, 1 - alpha);
}

void BiquadFilterNode::setAllpassCoefficients(
    size_t index,
    float frequency,
    float Q) {
  if (frequency <= 0.0 || frequency >= 1.0) {
    setNormalizedCoefficients(index, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  if (Q <= 0.0) {
    setNormalizedCoefficients(index, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    return;
  }

  float w0 = PI * frequency;
  float alpha = std::sin(w0) / (2 * Q);
  float cosW = std::cos(w0);

  setNormalizedCoefficients(
      index, 1 - alpha, -2 * cosW, 1 + alpha, 1 + alpha, -2 * cosW, 1 - alpha);
}

void BiquadFilterNode::updateCoefficients(
    size_t framesToProcess,
    float *frequency,
    float *detune,
    float *Q,
    float *gain) {
  for (size_t i = 0; i < framesToProcess; ++i) {
    float normalizedFrequency = frequency[i] / context_->getNyquistFrequency();

    if (detune[i] != 0.0f) {
      normalizedFrequency *= std::pow(2.0f, detune[i] / 1200.0f);
    }

    switch (type_) {
      case BiquadFilterType::LOWPASS:
        setLowpassCoefficients(i, normalizedFrequency, Q[i]);
        break;
      case BiquadFilterType::HIGHPASS:
        setHighpassCoefficients(i, normalizedFrequency, Q[i]);
        break;
      case BiquadFilterType::BANDPASS:
        setBandpassCoefficients(i, normalizedFrequency, Q[i]);
        break;
      case BiquadFilterType::LOWSHELF:
        setLowshelfCoefficients(i, normalizedFrequency, gain[i]);
        break;
      case BiquadFilterType::HIGHSHELF:
        setHighshelfCoefficients(i, normalizedFrequency, gain[i]);
        break;
      case BiquadFilterType::PEAKING:
        setPeakingCoefficients(i, normalizedFrequency, Q[i], gain[i]);
        break;
      case BiquadFilterType::NOTCH:
        setNotchCoefficients(i, normalizedFrequency, Q[i]);
        break;
      case BiquadFilterType::ALLPASS:
        setAllpassCoefficients(i, normalizedFrequency, Q[i]);
        break;
      default:
        break;
    }
  }
}

bool BiquadFilterNode::hasConstantValue(const AudioArray *values) const {
  const float value = (*values)[0];
  for (size_t i = 1; i < values->getSize(); ++i) {
    if ((*values)[i] != value) {
      return false;
    }
  }
  return true;
}

void BiquadFilterNode::processNode(
    const std::shared_ptr<AudioBus> &processingBus,
    int framesToProcess) {
  double currentTime = context_->getCurrentTime();
  auto frequencyValues =
      frequencyParam_->processARateParam(framesToProcess, currentTime)
          ->getChannel(0);
  auto detuneValues =
      detuneParam_->processARateParam(framesToProcess, currentTime)
          ->getChannel(0);
  auto qValues =
      QParam_->processARateParam(framesToProcess, currentTime)->getChannel(0);
  auto gainValues = gainParam_->processARateParam(framesToProcess, currentTime)
                        ->getChannel(0);

  for (int c = 0; c < processingBus->getNumberOfChannels(); ++c) {
    bool isConstant = hasConstantValue(frequencyValues) &&
        hasConstantValue(detuneValues) && hasConstantValue(qValues) &&
        hasConstantValue(gainValues);

    updateCoefficients(
        isConstant ? 1 : framesToProcess,
        frequencyValues->getData(),
        detuneValues->getData(),
        qValues->getData(),
        gainValues->getData());

    // Local copies for micro-optimization
    float x1 = x1_;
    float x2 = x2_;
    float y1 = y1_;
    float y2 = y2_;

    if (isConstant) {
      // Local copies for micro-optimization
      float b0 = b0_[0];
      float b1 = b1_[0];
      float b2 = b2_[0];
      float a1 = a1_[0];
      float a2 = a2_[0];

      for (int i = 0; i < framesToProcess; i++) {
        float input = (*processingBus->getChannel(c))[i];
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        (*processingBus->getChannel(c))[i] = output;

        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
      }
    } else {
      for (int i = 0; i < framesToProcess; i++) {
        float input = (*processingBus->getChannel(c))[i];
        float output = b0_[i] * input + b1_[i] * x1 + b2_[i] * x2 -
            a1_[i] * y1 - a2_[i] * y2;

        (*processingBus->getChannel(c))[i] = output;

        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
      }
    }
    x1_ = x1;
    x2_ = x2;
    y1_ = y1;
    y2_ = y2;
  }
}

} // namespace audioapi
