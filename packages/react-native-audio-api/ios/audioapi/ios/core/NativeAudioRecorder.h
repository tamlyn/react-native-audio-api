#pragma once

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

typedef void (^AudioReceiverBlock)(const AudioBufferList *inputBuffer, int numFrames);

@interface NativeAudioRecorder : NSObject

@property (nonatomic, assign) int bufferLength;
@property (nonatomic, assign) float sampleRate;

@property (nonatomic, strong) AVAudioSinkNode *sinkNode;
@property (nonatomic, copy) AVAudioSinkNodeReceiverBlock receiverSinkBlock;
@property (nonatomic, copy) AudioReceiverBlock receiverBlock;

- (instancetype)initWithReceiverBlock:(AudioReceiverBlock)receiverBlock;

- (AVAudioFormat *)getInputFormat;

- (void)start;

- (void)stop;

- (void)cleanup;

@end
