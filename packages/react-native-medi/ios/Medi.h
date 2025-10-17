#import <MediSpec/MediSpec.h>
#import <React/RCTBridgeModule.h>
#import <CoreMIDI/CoreMIDI.h>

@class MediEventEmitter;
@class RCTBridge;

@interface Medi : NSObject <NativeMediSpec, RCTBridgeModule>

@property (nonatomic, assign) MIDIClientRef midiClient;
@property (nonatomic, assign) MIDIPortRef midiInputPort;
@property (nonatomic, assign) MIDIPortRef midiOutputPort;
@property (nonatomic, strong) MediEventEmitter *eventEmitter;
@property (nonatomic, weak) RCTBridge *bridge;

@end
