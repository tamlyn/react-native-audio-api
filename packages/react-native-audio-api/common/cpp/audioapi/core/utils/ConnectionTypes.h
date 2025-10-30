#pragma once

#include <memory>

namespace audioapi {

class AudioNode;

struct InputConnection {
  AudioNode *sourceNode;
  unsigned int outputIndexFromSource;
};

struct OutputConnection {
  std::shared_ptr<AudioNode> destinationNode;
  unsigned int inputIndexAtDestination;
};

} // namespace audioapi
