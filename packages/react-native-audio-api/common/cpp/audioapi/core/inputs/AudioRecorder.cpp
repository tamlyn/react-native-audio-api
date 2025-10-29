// void AudioRecorder::connect(const std::shared_ptr<RecorderAdapterNode> &node)
// { node->init(); adapterNodeLock_.lock(); adapterNode_ = node;
// adapterNodeLock_.unlock();
// isConnected_.store(true);
//   node->init(ringBufferSize_);
//   adapterNodeLock_.lock();
//   adapterNode_ = node;
//   adapterNodeLock_.unlock();
// }

// void AudioRecorder::disconnect() {
// adapterNodeLock_.lock();
// adapterNode_ = nullptr;
// adapterNodeLock_.unlock();
// isConnected_.store(false);
// }

// AudioRecorder::AudioRecorder(
//     const std::shared_ptr<AudioEventHandlerRegistry>
//     &audioEventHandlerRegistry)
//       audioEventHandlerRegistry_(audioEventHandlerRegistry) {
//   constexpr int minRingBufferSize = 8192;
//   ringBufferSize_ = std::max(2 * bufferLength, minRingBufferSize);

//   circularBuffer_ = std::make_shared<CircularAudioArray>(ringBufferSize_);
//   isRunning_.store(false);
// }

// void AudioRecorder::writeToBuffers(const float *data, int numFrames) {
//   if (adapterNodeLock_.try_lock()) {
//     if (adapterNode_ != nullptr) {
//       adapterNode_->buff_->write(data, numFrames);
//     }
//     adapterNodeLock_.unlock();
//   }
// }
