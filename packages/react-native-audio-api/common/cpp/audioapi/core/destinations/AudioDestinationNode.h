#pragma once

#include <audioapi/core/AudioNode.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

namespace audioapi {

class AudioBus;
class BaseAudioContext;

class AudioDestinationNode : public AudioNode {
 public:
  explicit AudioDestinationNode(BaseAudioContext *context);
  std::size_t getCurrentSampleFrame() const;
  double getCurrentTime() const;

  void renderAudio(const std::shared_ptr<AudioBus> &audioData, int numFrames);

 protected:
  // DestinationNode is triggered by AudioContext using renderAudio
  // processNode function is not necessary and is never called.
  void processNode(const std::vector<std::shared_ptr<AudioBus>> &inputBuses, int framesToProcess) final {
    return;
  }

 private:
  std::size_t currentSampleFrame_;
};

} // namespace audioapi
