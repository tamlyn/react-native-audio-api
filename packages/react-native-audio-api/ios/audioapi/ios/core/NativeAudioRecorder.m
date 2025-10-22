#import <audioapi/ios/core/NativeAudioRecorder.h>
#import <audioapi/ios/system/AudioEngine.h>
#import <audioapi/ios/system/AudioSessionManager.h>

@implementation NativeAudioRecorder

- (instancetype)initWithReceiverBlock:(AudioReceiverBlock)receiverBlock
{
  if (self = [super init]) {
    self.receiverBlock = [receiverBlock copy];

    __weak typeof(self) weakSelf = self;
    self.receiverSinkBlock = ^OSStatus(
        const AudioTimeStamp *_Nonnull timestamp,
        AVAudioFrameCount frameCount,
        const AudioBufferList *_Nonnull inputData) {
      weakSelf.receiverBlock(inputData, frameCount);

      return kAudioServicesNoError;
    };

    self.sinkNode = [[AVAudioSinkNode alloc] initWithReceiverBlock:self.receiverSinkBlock];
  }

  return self;
}

- (AVAudioFormat *)getInputFormat
{
  return [AudioEngine.sharedInstance.audioEngine.inputNode inputFormatForBus:0];
}

- (void)start
{
  AudioEngine *audioEngine = [AudioEngine sharedInstance];
  assert(audioEngine != nil);

  // AudioEngine allows us to attach and connect nodes at runtime but with few limitations
  // in this case if it is the first recorder node and player started the engine we need to restart.
  // It can be optimized by tracking if we haven't break rules of at runtime modifications from docs
  // https://developer.apple.com/documentation/avfaudio/avaudioengine?language=objc
  //
  // Currently we are restarting because we do not see any significant performance issue and case when
  // you will need to start and stop recorder very frequently
  [audioEngine stopEngine];
  [audioEngine attachInputNode:self.sinkNode];
  [audioEngine startIfNecessary];
}

- (void)stop
{
  AudioEngine *audioEngine = [AudioEngine sharedInstance];
  assert(audioEngine != nil);
  [audioEngine detachInputNode];
  [audioEngine stopIfNecessary];
}

- (void)cleanup
{
  self.receiverBlock = nil;
}

@end
