#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/HostObjects/sources/AudioBufferHostObject.h>
#include <audioapi/core/utils/Constants.h>
#include <audioapi/dsp/VectorMath.h>
#include <audioapi/events/AudioEventHandlerRegistry.h>
#include <audioapi/ios/core/utils/RecorderCallback.h>
#include <audioapi/utils/AudioArray.h>
#include <audioapi/utils/AudioBus.h>
#include <audioapi/utils/CircularAudioArray.h>
#include <audioapi/utils/Result.hpp>
#include <algorithm>

namespace audioapi {

RecorderCallback::RecorderCallback(
    const std::shared_ptr<AudioEventHandlerRegistry> &audioEventHandlerRegistry,
    float sampleRate,
    size_t bufferLength,
    int channelCount,
    uint64_t callbackId)
    : audioEventHandlerRegistry_(audioEventHandlerRegistry),
      sampleRate_(sampleRate),
      bufferLength_(bufferLength),
      channelCount_(channelCount),
      callbackId_(callbackId)
{
  ringBufferSize_ = std::max((int)bufferLength_ * 2, 8192);
  circularBus_.resize(channelCount_);

  for (int i = 0; i < channelCount_; ++i) {
    auto busAudioArray = std::make_shared<CircularAudioArray>(ringBufferSize_);
    circularBus_[i] = busAudioArray;
  }

  isInitialized_.store(true, std::memory_order_release);
}

RecorderCallback::~RecorderCallback()
{
  isInitialized_.store(false, std::memory_order_release);
  @autoreleasepool {
    converter_ = nil;
    bufferFormat_ = nil;
    callbackFormat_ = nil;
    converterInputBuffer_ = nil;
    converterOutputBuffer_ = nil;

    for (int i = 0; i < channelCount_; ++i) {
      circularBus_[i].reset();
    }
  }
}

Result<NoneType, std::string> RecorderCallback::prepare(
    AVAudioFormat *bufferFormat,
    size_t maxInputBufferLength)
{
  @autoreleasepool {
    bufferFormat_ = bufferFormat;
    converterInputBufferSize_ = maxInputBufferLength;

    if (bufferFormat.sampleRate <= 0 || bufferFormat.channelCount == 0) {
      return Result<NoneType, std::string>::Err(
          "Invalid input format: sampleRate and channelCount must be greater than 0");
    }

    if (sampleRate_ <= 0 || channelCount_ == 0) {
      return Result<NoneType, std::string>::Err(
          "Invalid callback format: sampleRate and channelCount must be greater than 0");
    }

    converterOutputBufferSize_ = std::max(
        (double)maxInputBufferLength, sampleRate_ / bufferFormat.sampleRate * maxInputBufferLength);

    callbackFormat_ = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatFloat32
                                                       sampleRate:sampleRate_
                                                         channels:channelCount_
                                                      interleaved:NO];

    converter_ = [[AVAudioConverter alloc] initFromFormat:bufferFormat toFormat:callbackFormat_];
    converter_.sampleRateConverterAlgorithm = AVSampleRateConverterAlgorithm_Normal;
    converter_.sampleRateConverterQuality = AVAudioQualityMax;
    converter_.primeMethod = AVAudioConverterPrimeMethod_None;

    converterInputBuffer_ =
        [[AVAudioPCMBuffer alloc] initWithPCMFormat:bufferFormat_
                                      frameCapacity:(AVAudioFrameCount)converterInputBufferSize_];
    converterOutputBuffer_ =
        [[AVAudioPCMBuffer alloc] initWithPCMFormat:callbackFormat_
                                      frameCapacity:(AVAudioFrameCount)converterOutputBufferSize_];
  }

  return Result<NoneType, std::string>::Ok(None);
}

void RecorderCallback::cleanup()
{
  @autoreleasepool {
    emitAudioData(true);

    converter_ = nil;
    bufferFormat_ = nil;
    callbackFormat_ = nil;
    converterInputBuffer_ = nil;
    converterOutputBuffer_ = nil;

    for (int i = 0; i < channelCount_; ++i) {
      circularBus_[i]->zero();
    }
  }
}

void RecorderCallback::receiveAudioData(const AudioBufferList *inputBuffer, int numFrames)
{
  if (!isInitialized_.load(std::memory_order_acquire)) {
    return;
  }

  @autoreleasepool {
    NSError *error = nil;

    if (bufferFormat_.sampleRate == sampleRate_ && bufferFormat_.channelCount == channelCount_ &&
        !bufferFormat_.isInterleaved) {
      // Directly write to circular buffer
      for (int i = 0; i < channelCount_; ++i) {
        auto *inputChannel = static_cast<float *>(inputBuffer->mBuffers[i].mData);
        circularBus_[i]->push_back(inputChannel, numFrames);
      }

      emitAudioData();
      return;
    }

    size_t outputFrameCount = ceil(numFrames * (sampleRate_ / bufferFormat_.sampleRate));

    for (size_t i = 0; i < bufferFormat_.channelCount; ++i) {
      memcpy(
          converterInputBuffer_.mutableAudioBufferList->mBuffers[i].mData,
          inputBuffer->mBuffers[i].mData,
          inputBuffer->mBuffers[i].mDataByteSize);
    }

    converterInputBuffer_.frameLength = numFrames;

    __block BOOL handedOff = false;
    AVAudioConverterInputBlock inputBlock = ^AVAudioBuffer *_Nullable(
        AVAudioPacketCount inNumberOfPackets, AVAudioConverterInputStatus *outStatus)
    {
      if (handedOff) {
        *outStatus = AVAudioConverterInputStatus_NoDataNow;
        return nil;
      }

      handedOff = true;
      *outStatus = AVAudioConverterInputStatus_HaveData;
      return converterInputBuffer_;
    };

    [converter_ convertToBuffer:converterOutputBuffer_ error:&error withInputFromBlock:inputBlock];
    converterOutputBuffer_.frameLength = sampleRate_ / bufferFormat_.sampleRate * numFrames;

    if (error != nil) {
      invokeOnErrorCallback(
          std::string("Error during audio conversion, native error: ") +
          [[error debugDescription] UTF8String]);
      return;
    }

    for (int i = 0; i < channelCount_; ++i) {
      auto *inputChannel =
          static_cast<float *>(converterOutputBuffer_.audioBufferList->mBuffers[i].mData);
      circularBus_[i]->push_back(inputChannel, outputFrameCount);
    }

    emitAudioData();
  }
}

void RecorderCallback::emitAudioData(bool flush)
{
  size_t sizeLimit = flush ? circularBus_[0]->getNumberOfAvailableFrames() : bufferLength_;

  while (circularBus_[0]->getNumberOfAvailableFrames() >= sizeLimit) {
    auto bus = std::make_shared<AudioBus>(sizeLimit, channelCount_, sampleRate_);

    for (int i = 0; i < channelCount_; ++i) {
      auto *outputChannel = bus->getChannel(i)->getData();
      circularBus_[i]->pop_front(outputChannel, sizeLimit);
    }

    invokeCallback(bus, sizeLimit);
  }
}

void RecorderCallback::invokeCallback(const std::shared_ptr<AudioBus> &bus, int numFrames)
{
  auto audioBuffer = std::make_shared<AudioBuffer>(bus);
  auto audioBufferHostObject = std::make_shared<AudioBufferHostObject>(audioBuffer);

  std::unordered_map<std::string, EventValue> eventPayload = {};
  eventPayload.insert({"buffer", audioBufferHostObject});
  eventPayload.insert({"numFrames", numFrames});

  if (audioEventHandlerRegistry_) {
    audioEventHandlerRegistry_->invokeHandlerWithEventBody("audioReady", callbackId_, eventPayload);
  }
}

void RecorderCallback::setOnErrorCallback(uint64_t callbackId)
{
  errorCallbackId_.store(callbackId, std::memory_order_release);
}

void RecorderCallback::clearOnErrorCallback()
{
  errorCallbackId_.store(0, std::memory_order_release);
}

void RecorderCallback::invokeOnErrorCallback(const std::string &message)
{
  uint64_t callbackId = errorCallbackId_.load(std::memory_order_acquire);

  // TODO: only the line above is atomic, which means that between reading the callbackId and invoking the callback,
  // the callback could be cleared. We need to ensure that the callback is still valid when invoking it.
  // TL;DR: atomic szpont
  if (audioEventHandlerRegistry_ == nullptr || callbackId == 0) {
    return;
  }

  std::unordered_map<std::string, EventValue> eventPayload = {};
  eventPayload.insert({"message", message});
  audioEventHandlerRegistry_->invokeHandlerWithEventBody("error", callbackId, eventPayload);
}

} // namespace audioapi
