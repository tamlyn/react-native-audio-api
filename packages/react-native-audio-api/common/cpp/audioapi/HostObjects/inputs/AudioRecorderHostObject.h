#pragma once

#include <audioapi/jsi/JsiHostObject.h>

#include <memory>
#include <utility>
#include <vector>
#include <cstdio>
#include <string>

namespace audioapi {
using namespace facebook;

class AudioRecorder;
class AudioEventHandlerRegistry;

class AudioRecorderHostObject : public JsiHostObject {
 public:
  explicit AudioRecorderHostObject(
      const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry);

  JSI_HOST_FUNCTION_DECL(start);
  JSI_HOST_FUNCTION_DECL(stop);
  JSI_HOST_FUNCTION_DECL(isRecording);
  JSI_HOST_FUNCTION_DECL(isPaused);

  JSI_HOST_FUNCTION_DECL(enableFileOutput);
  JSI_HOST_FUNCTION_DECL(disableFileOutput);

  JSI_HOST_FUNCTION_DECL(pause);
  JSI_HOST_FUNCTION_DECL(resume);

  JSI_HOST_FUNCTION_DECL(connect);
  JSI_HOST_FUNCTION_DECL(disconnect);

  JSI_HOST_FUNCTION_DECL(setOnAudioReady);
  JSI_HOST_FUNCTION_DECL(clearOnAudioReady);

  JSI_HOST_FUNCTION_DECL(getCurrentDuration);

 private:
  std::shared_ptr<AudioRecorder> audioRecorder_;
};

} // namespace audioapi
