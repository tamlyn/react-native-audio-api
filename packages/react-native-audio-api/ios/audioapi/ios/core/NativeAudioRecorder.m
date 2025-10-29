#import <audioapi/ios/core/NativeAudioRecorder.h>
#import <audioapi/ios/system/AudioEngine.h>
#import <audioapi/ios/system/AudioSessionManager.h>

@implementation NativeAudioRecorder

static inline uint32_t nextPowerOfTwo(uint32_t x)
{
  if (x == 0) {
    return 1;
  }

  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;

  return x;
}

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

- (int)getBufferSize
{
  // NOTE: this method should be called only after the session is activated
  AVAudioSession *audioSession = [AVAudioSession sharedInstance];
  float bufferDuration = audioSession.IOBufferDuration > 0 ? audioSession.IOBufferDuration : 0.02;

  return nextPowerOfTwo(ceil(bufferDuration * audioSession.sampleRate));
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
