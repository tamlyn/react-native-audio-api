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
    size_t outputIndex = args[1].asNumber();
    size_t inputIndex = args[2].asNumber();
    node_->connect(
        std::shared_ptr<AudioNodeHostObject>(node)->node_,
        outputIndex,
        inputIndex);
  }
  if (obj.isHostObject<AudioParamHostObject>(runtime)) {
    auto param = obj.getHostObject<AudioParamHostObject>(runtime);
    size_t outputIndex = args[1].asNumber();
    node_->connect(
        std::shared_ptr<AudioParamHostObject>(param)->param_, outputIndex);
  }
  return jsi::Value::undefined();
}

JSI_HOST_FUNCTION_IMPL(AudioNodeHostObject, disconnect) {
  auto throwInvalidAccessError = [&](const std::string &message) {
    throw jsi::JSError(runtime, "InvalidAccessError: " + message);
  };

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
        // TS layer handled IndexSizeError
        node_->disconnect(static_cast<unsigned int>(arg.asNumber()));
      } else {
        auto obj = arg.asObject(runtime);
        if (obj.isHostObject<AudioNodeHostObject>(runtime)) {
          // Signature: disconnect(destinationNode)
          auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);
          bool disconnected = node_->disconnect(destHostObject->node_);
          if (!disconnected) {
            throwInvalidAccessError(
                "Failed to execute 'disconnect' on 'AudioNode': "
                "the specified destination node is not connected to this node.");
          }
        } else {
          auto destHostObject =
              obj.getHostObject<AudioParamHostObject>(runtime);
          bool disconnected = node_->disconnect(destHostObject->param_);
          if (!disconnected) {
            throwInvalidAccessError(
                "Failed to execute 'disconnect' on 'AudioNode': "
                "the specified destination parameter is not connected to this node.");
          }
        }
      }
      break;
    }

    case 2: {
      auto obj = args[0].asObject(runtime);
      auto outputIndex = static_cast<unsigned int>(args[1].asNumber());

      if (obj.isHostObject<AudioNodeHostObject>(runtime)) {
        // Signature: disconnect(destinationNode, outputIndex)
        auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);

        bool disconnected =
            node_->disconnect(destHostObject->node_, outputIndex);
        if (!disconnected) {
          throwInvalidAccessError(
              "Failed to execute 'disconnect' on 'AudioNode': "
              "no connection was found from the given output to the specified AudioNode.");
        }

      } else {
        auto destHostObject = obj.getHostObject<AudioParamHostObject>(runtime);

        bool disconnected =
            node_->disconnect(destHostObject->param_, outputIndex);
        if (!disconnected) {
          throwInvalidAccessError(
              "Failed to execute 'disconnect' on 'AudioNode': "
              "no connection was found from the given output to the specified AudioParam.");
        }
      }
      break;
    }

    case 3: {
      auto obj = args[0].asObject(runtime);
      auto destHostObject = obj.getHostObject<AudioNodeHostObject>(runtime);
      auto outputIndex = static_cast<unsigned int>(args[1].asNumber());
      auto inputIndex = static_cast<unsigned int>(args[2].asNumber());

      // Signature: disconnect(destinationNode, outputIndex, inputIndex)
      bool disconnected =
          node_->disconnect(destHostObject->node_, outputIndex, inputIndex);
      if (!disconnected) {
        throwInvalidAccessError(
            "Failed to execute 'disconnect' on 'AudioNode': "
            "no connection was found from the given output to the given input of the specified AudioNode.");
      }
      break;
    }

    default: {
      throw jsi::JSError(runtime, "disconnect: Invalid number of arguments.");
    }
  }

  return jsi::Value::undefined();
}
} // namespace audioapi
