#import "MediEventEmitter.h"

// Web MIDI API compliant event names
NSString *const MediEventMIDIMessage = @"onMIDIMessage";
NSString *const MediEventStateChange = @"onStateChange";

@implementation MediEventEmitter

RCT_EXPORT_MODULE();

+ (BOOL)requiresMainQueueSetup {
    return NO;
}

- (instancetype)init {
    if (self = [super init]) {
        _listenedPortIds = [NSMutableDictionary dictionary];
    }
    return self;
}

- (NSArray<NSString *> *)supportedEvents {
    return @[MediEventMIDIMessage, MediEventStateChange];
}

- (void)startObservingPortId:(NSString *)portId {
    if (!portId) return;

    NSNumber *currentCount = self.listenedPortIds[portId];
    NSInteger count = currentCount ? [currentCount integerValue] : 0;
    self.listenedPortIds[portId] = @(count + 1);
}

- (void)stopObservingPortId:(NSString *)portId {
    if (!portId) return;

    NSNumber *currentCount = self.listenedPortIds[portId];
    if (!currentCount) return;

    NSInteger count = [currentCount integerValue];
    if (count <= 1) {
        [self.listenedPortIds removeObjectForKey:portId];
    } else {
        self.listenedPortIds[portId] = @(count - 1);
    }
}

- (BOOL)hasListenersForPortId:(NSString *)portId {
    if (!portId) return NO;

    NSNumber *count = self.listenedPortIds[portId];
    return count && [count integerValue] > 0;
}

#pragma mark - Event Emission

- (void)sendMIDIMessageEvent:(NSString *)portId
                         data:(NSArray<NSNumber *> *)data
                    timestamp:(double)timestamp {
    // Only send event if there are active listeners for this port
    if (![self hasListenersForPortId:portId]) {
        return;
    }

    [self sendEventWithName:MediEventMIDIMessage body:@{
        @"portId": portId,
        @"data": data,
        @"timestamp": @(timestamp)
    }];
}

- (void)sendStateChangeEvent:(NSString *)portId
                        state:(NSString *)state
                   connection:(NSString *)connection {
    // State change events should always be sent (device add/remove notifications)
    // These are not port-specific message events, so we broadcast them
    [self sendEventWithName:MediEventStateChange body:@{
        @"portId": portId,
        @"state": state,
        @"connection": connection
    }];
}

@end
