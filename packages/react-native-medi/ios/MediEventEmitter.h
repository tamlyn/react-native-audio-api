#import <React/RCTEventEmitter.h>
#import <React/RCTBridgeModule.h>

// Web MIDI API compliant event names
extern NSString *const MediEventMIDIMessage;
extern NSString *const MediEventStateChange;

@interface MediEventEmitter : RCTEventEmitter <RCTBridgeModule>

// Map of port IDs to listener counts
// Key: portId (NSString), Value: listener count (NSNumber)
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSNumber *> *listenedPortIds;

- (void)startObservingPortId:(NSString *)portId;
- (void)stopObservingPortId:(NSString *)portId;
- (BOOL)hasListenersForPortId:(NSString *)portId;

// Send MIDI message event (matches Web MIDI 'midimessage' event)
- (void)sendMIDIMessageEvent:(NSString *)portId
                         data:(NSArray<NSNumber *> *)data
                    timestamp:(double)timestamp;

// Send state change event (matches Web MIDI 'statechange' event)
- (void)sendStateChangeEvent:(NSString *)portId
                        state:(NSString *)state
                   connection:(NSString *)connection;

@end
