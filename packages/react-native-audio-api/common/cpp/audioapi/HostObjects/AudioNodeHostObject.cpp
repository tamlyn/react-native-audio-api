#include <audioapi/HostObjects/AudioNodeHostObject.h>

#include <audioapi/HostObjects/AudioParamHostObject.h>
#include <audioapi/core/AudioNode.h>

namespace audioapi {

AudioNodeHostObject::AudioNodeHostObject(const std::shared_ptr<AudioNode> &node)
    : node_(node) {
  addGetters(
      JSI_EXPORT_PROPERTY_GETTER(AudioNodeHostObject, numberOfInputs),
      JSI_EXPORT_PROPERTY_GETTER(AudioNodeHostObject, numberOfOutputs),
      JSI_EXPORT_PROPERTY_GETTER(AudioNodeHostObject, channelCount),
      JSI_EXPORT_PROPERTY_GETTER(AudioNodeHostObject, channelCountMode),
      JSI_EXPORT_PROPERTY_GETTER(AudioNodeHostObject, channelInterpretation));

  addFunctions(
      JSI_EXPORT_FUNCTION(AudioNodeHostObject, connect),
      JSI_EXPORT_FUNCTION(AudioNodeHostObject, disconnect));
}

JSI_PROPERTY_GETTER_IMPL(AudioNodeHostObject, numberOfInputs) {
  return {node_->getNumberOfInputs()};
}

JSI_PROPERTY_GETTER_IMPL(AudioNodeHostObject, numberOfOutputs) {
  return {node_->getNumberOfOutputs()};
}

JSI_PROPERTY_GETTER_IMPL(AudioNodeHostObject, channelCount) {
  return {node_->getChannelCount()};
}

JSI_PROPERTY_GETTER_IMPL(AudioNodeHostObject, channelCountMode) {
  return jsi::String::createFromUtf8(runtime, node_->getChannelCountMode());
}

JSI_PROPERTY_GETTER_IMPL(AudioNodeHostObject, channelInterpretation) {
  return jsi::String::createFromUtf8(
      runtime, node_->getChannelInterpretation());
}

JSI_HOST_FUNCTION_IMPL(AudioNodeHostObject, connect) {
  auto obj = args[0].getObject(runtime);

  if (obj.isHostObject<AudioNodeHostObject>(runtime)) {
    auto node = obj.getHostObject<AudioNodeHostObject>(runtime);
    unsigned int outputIndex = args[1].asNumber();
    unsigned int inputIndex = args[2].asNumber();
    node_->connect(
        std::shared_ptr<AudioNodeHostObject>(node)->node_,
        outputIndex,
        inputIndex);
  }
  if (obj.isHostObject<AudioParamHostObject>(runtime)) {
    auto param = obj.getHostObject<AudioParamHostObject>(runtime);
    unsigned int outputIndex = args[1].asNumber();
    node_->connect(
        std::shared_ptr<AudioParamHostObject>(param)->param_, outputIndex);
  }
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioNodeHostObject, disconnect) {
  switch (count) {
    case 0: {
      // Signature: disconnect()
      node_->disconnect();
      break;
    }

    case 1: {
      const auto &arg = args[0];
      if (arg.isNumber()) {
        // Signature: disconnect(outputIndex)
        node_->disconnect(static_cast<unsigned int>(arg.asNumber()));
      } else if (arg.isObject()) {
        auto obj = arg.asObject(runtime);
        if (obj.isHostObject<AudioNodeHostObject>(runtime)) {
          // Signature: disconnect(destinationNode)
          auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);
          node_->disconnect(destHostObject->node_);
        } else if (obj.isHostObject<AudioParamHostObject>(runtime)) {
          // Signature: disconnect(destinationParam)
          auto destHostObject =
              obj.getHostObject<AudioParamHostObject>(runtime);
          node_->disconnect(destHostObject->param_);
        } else {
          throw jsi::JSError(
              runtime,
              "disconnect: Argument 1 must be a number, AudioNode, or AudioParam.");
        }
      } else {
        throw jsi::JSError(runtime, "disconnect: Invalid argument.");
      }
      break;
    }

    case 2: {
      if (!args[0].isObject() || !args[1].isNumber()) {
        throw jsi::JSError(
            runtime,
            "disconnect: Expected arguments of type (AudioNode | AudioParam, number).");
      }
      auto obj = args[0].asObject(runtime);
      auto outputIndex = static_cast<unsigned int>(args[1].asNumber());
      if (obj.isHostObject<AudioNodeHostObject>(runtime)) {
        // Signature: disconnect(destinationNode, outputIndex)
        auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);
        node_->disconnect(destHostObject->node_, outputIndex);
      } else if (obj.isHostObject<AudioParamHostObject>(runtime)) {
        // Signature: disconnect(destinationParam, outputIndex)
        auto destHostObject = obj.getHostObject<AudioParamHostObject>(runtime);
        node_->disconnect(destHostObject->param_, outputIndex);
      } else {
        throw jsi::JSError(
            runtime,
            "disconnect: Argument 1 must be an AudioNode or AudioParam.");
      }
      break;
    }

    case 3: {
      if (!args[0].isObject() || !args[1].isNumber() || !args[2].isNumber()) {
        throw jsi::JSError(
            runtime,
            "disconnect: Expected arguments of type (AudioNode, number, number).");
      }
      auto obj = args[0].asObject(runtime);
      if (!obj.isHostObject<AudioNodeHostObject>(runtime)) {
        throw jsi::JSError(
            runtime,
            "disconnect: With 3 arguments, the first must be an AudioNode.");
      }
      auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);
      auto outputIndex = static_cast<unsigned int>(args[1].asNumber());
      auto inputIndex = static_cast<unsigned int>(args[2].asNumber());
      // Signature: disconnect(destinationNode, outputIndex, inputIndex)
      node_->disconnect(destHostObject->node_, outputIndex, inputIndex);
      break;
    }

    default: {
      throw jsi::JSError(runtime, "disconnect: Invalid number of arguments.");
    }
  }

  return jsi::Value::undefined();
}
} // namespace audioapi
