
#include <audioapi/core/inputs/AudioRecorder.h>
#include <audioapi/core/utils/AudioFileWriter.h>
#include <audioapi/core/utils/AudioRecorderCallback.h>

namespace audioapi {

void AudioRecorder::setOnErrorCallback(uint64_t callbackId) {
  std::scoped_lock lock(callbackMutex_, fileWriterMutex_, errorCallbackMutex_);

  if (usesFileOutput()) {
    fileWriter_->setOnErrorCallback(callbackId);
  }

  if (usesCallback()) {
    dataCallback_->setOnErrorCallback(callbackId);
  }

  errorCallbackId_.store(callbackId, std::memory_order_release);
}

void AudioRecorder::clearOnErrorCallback() {
  std::scoped_lock lock(callbackMutex_, fileWriterMutex_, errorCallbackMutex_);

  if (usesFileOutput()) {
    fileWriter_->clearOnErrorCallback();
  }

  if (usesCallback()) {
    dataCallback_->clearOnErrorCallback();
  }

  errorCallbackId_.store(0, std::memory_order_release);
}

double AudioRecorder::getCurrentDuration() const {
  double duration = 0.0;

  if (usesFileOutput()) {
    duration = fileWriter_->getCurrentDuration();
  }

  return duration;
}

bool AudioRecorder::usesCallback() const {
  return callbackOutputEnabled_.load(std::memory_order_acquire);
}

bool AudioRecorder::usesFileOutput() const {
  return fileOutputEnabled_.load(std::memory_order_acquire);
}

bool AudioRecorder::isConnected() const {
  return isConnected_.load(std::memory_order_acquire);
}

} // namespace audioapi
