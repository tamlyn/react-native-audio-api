#include <audioapi/HostObjects/inputs/AudioRecorderHostObject.h>

#include <audioapi/HostObjects/sources/AudioBufferHostObject.h>
#include <audioapi/HostObjects/sources/RecorderAdapterNodeHostObject.h>
#include <audioapi/core/inputs/AudioRecorder.h>
#include <audioapi/core/sources/AudioBuffer.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#ifdef ANDROID
#include <audioapi/android/core/AndroidAudioRecorder.h>
#else
#include <audioapi/ios/core/IOSAudioRecorder.h>
#endif

namespace audioapi {

AudioRecorderHostObject::AudioRecorderHostObject(
    const std::shared_ptr<AudioEventHandlerRegistry>
        &audioEventHandlerRegistry) {
#ifdef ANDROID
  audioRecorder_ =
      std::make_shared<AndroidAudioRecorder>(audioEventHandlerRegistry);
#else
  audioRecorder_ =
      std::make_shared<IOSAudioRecorder>(audioEventHandlerRegistry);
#endif

  addFunctions(
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, start),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, stop),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, isRecording),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, enableFileOutput),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, disableFileOutput),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, pause),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, resume),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, connect),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, disconnect),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, setOnAudioReady),
      JSI_EXPORT_FUNCTION(AudioRecorderHostObject, clearOnAudioReady));
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, start) {
  audioRecorder_->start();

  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, stop) {
  std::string filePath = audioRecorder_->stop();

  return jsi::Value(runtime, jsi::String::createFromUtf8(runtime, filePath));
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, isRecording) {
  return jsi::Value(audioRecorder_->isRecording());
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, enableFileOutput) {
  auto options = args[0].getObject(runtime);

  auto sampleRate = static_cast<float>(
      options.getProperty(runtime, "sampleRate").getNumber());
  auto channelCount =
      static_cast<size_t>(options.getProperty(runtime, "channels").getNumber());
  auto bitRate =
      static_cast<size_t>(options.getProperty(runtime, "bitRate").getNumber());
  auto iosFlags =
      static_cast<size_t>(options.getProperty(runtime, "ios").getNumber());
  auto androidFlags =
      static_cast<size_t>(options.getProperty(runtime, "android").getNumber());

  audioRecorder_->enableFileOutput(
      sampleRate, channelCount, bitRate, iosFlags, androidFlags);
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, disableFileOutput) {
  audioRecorder_->disableFileOutput();
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, pause) {
  audioRecorder_->pause();

  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, resume) {
  audioRecorder_->resume();
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, connect) {
  auto adapterNodeHostObject =
      args[0].getObject(runtime).getHostObject<RecorderAdapterNodeHostObject>(
          runtime);

  audioRecorder_->connect(
      std::static_pointer_cast<RecorderAdapterNode>(
          adapterNodeHostObject->node_));
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, disconnect) {
  audioRecorder_->disconnect();

  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, setOnAudioReady) {
  auto options = args[0].getObject(runtime);

  auto sampleRate = static_cast<float>(
      options.getProperty(runtime, "sampleRate").getNumber());
  auto bufferLength = static_cast<size_t>(
      options.getProperty(runtime, "bufferLength").getNumber());
  auto channelCount = static_cast<size_t>(
      options.getProperty(runtime, "channelCount").getNumber());
  auto callbackId = static_cast<uint64_t>(
      options.getProperty(runtime, "callbackId").getNumber());

  audioRecorder_->setOnAudioReadyCallback(
      sampleRate, bufferLength, channelCount, callbackId);
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioRecorderHostObject, clearOnAudioReady) {
  audioRecorder_->clearOnAudioReadyCallback();
  return jsi::Value::undefined();
}

} // namespace audioapi
