#import "Medi.h"
#import "MediEventEmitter.h"
#import "MediPortManager.h"
#import <React/RCTBridge.h>

// Context structure to pass to MIDI read callback
typedef struct {
    void *portId;  // retained NSString
    void *emitter; // unretained MediEventEmitter
} MIDIReadContext;

// MIDI notification callback for device changes
static void MediNotificationCallback(const MIDINotification *message, void *refCon) {
    NSLog(@"🎹 MediNotificationCallback fired! messageID=%d", message->messageID);
}

// MIDI read callback for receiving messages (Web MIDI 'midimessage' event)
static void MediReadCallback(const MIDIPacketList *packetList, void *readProcefCon, void *srcConnRefCon) {
    MIDIReadContext *context = (MIDIReadContext *)srcConnRefCon;
    if (!context) {
        NSLog(@"MediReadCallback: context is nil");
        return;
    }

    // Use __bridge (not __bridge_transfer) to keep object alive across multiple callbacks
    NSString *portId = (__bridge NSString *)context->portId;
    MediEventEmitter *emitter = (__bridge MediEventEmitter *)context->emitter;

    if (!portId || !emitter) {
        NSLog(@"MediReadCallback: portId or emitter is nil");
        return;
    }

    const MIDIPacket *packet = &packetList->packet[0];
    for (UInt32 i = 0; i < packetList->numPackets; i++) {
        NSMutableArray *data = [NSMutableArray array];
        for (UInt16 j = 0; j < packet->length; j++) {
            [data addObject:@(packet->data[j])];
        }

        // Convert MIDITimeStamp to milliseconds (Web MIDI uses DOMHighResTimeStamp)
        double timestamp = (double)packet->timeStamp / 1000000.0;

        [emitter sendMIDIMessageEvent:portId
                                 data:data
                            timestamp:timestamp];

        packet = MIDIPacketNext(packet);
    }
}

@implementation Medi {
    MediPortManager *_portManager;
}

@synthesize bridge = _bridge;

- (instancetype)init {
    if (self = [super init]) {
        _portManager = nil;
    }
    return self;
}

- (void)setBridge:(RCTBridge *)bridge {
    _bridge = bridge;
    // Get the event emitter module from the bridge
    self.eventEmitter = [bridge moduleForClass:[MediEventEmitter class]];
}

- (void)dealloc {
    if (_midiInputPort) {
        MIDIPortDispose(_midiInputPort);
    }
    if (_midiOutputPort) {
        MIDIPortDispose(_midiOutputPort);
    }
    if (_midiClient) {
        MIDIClientDispose(_midiClient);
    }
}

#pragma mark - MIDI Access

/// Initialize MIDI system and request access
/// This corresponds to navigator.requestMIDIAccess() in Web MIDI API but does not need to be called every time
/// it returns true immediately if already initialized
- (void)requestMIDIAccess:(BOOL)sysex
                  resolve:(RCTPromiseResolveBlock)resolve
                   reject:(RCTPromiseRejectBlock)reject {
    if (_midiClient) {
        resolve(@YES);
        return;
    }

    CFStringRef clientName = CFSTR("ReactNativeMedi");
    OSStatus status = MIDIClientCreate(clientName, MediNotificationCallback, (__bridge void *)self, &_midiClient);

    if (status != noErr) {
        reject(@"MIDI_ERROR", @"Failed to create MIDI client", nil);
        return;
    }

    // Create input port for receiving MIDI messages (Web MIDI input)
    CFStringRef inputPortName = CFSTR("ReactNativeMedi Input");
    status = MIDIInputPortCreate(_midiClient, inputPortName, MediReadCallback, NULL, &_midiInputPort);

    if (status != noErr) {
        reject(@"MIDI_ERROR", @"Failed to create MIDI input port", nil);
        return;
    }

    // Create output port for sending MIDI messages (Web MIDI output)
    CFStringRef outputPortName = CFSTR("ReactNativeMedi Output");
    status = MIDIOutputPortCreate(_midiClient, outputPortName, &_midiOutputPort);

    if (status != noErr) {
        reject(@"MIDI_ERROR", @"Failed to create MIDI output port", nil);
        return;
    }

    // Initialize port manager
    _portManager = [[MediPortManager alloc] initWithClient:_midiClient
                                                 inputPort:_midiInputPort
                                                outputPort:_midiOutputPort
                                              eventEmitter:self.eventEmitter];

    resolve(@YES);
}

#pragma mark - Device Enumeration

- (void)getInputPorts:(RCTPromiseResolveBlock)resolve
               reject:(RCTPromiseRejectBlock)reject {
    if (!_portManager) {
        reject(@"MIDI_ERROR", @"MIDI not initialized. Call requestMIDIAccess first.", nil);
        return;
    }

    resolve([_portManager getInputPorts]);
}

- (void)getOutputPorts:(RCTPromiseResolveBlock)resolve
                reject:(RCTPromiseRejectBlock)reject {
    if (!_portManager) {
        reject(@"MIDI_ERROR", @"MIDI not initialized. Call requestMIDIAccess first.", nil);
        return;
    }

    resolve([_portManager getOutputPorts]);
}

#pragma mark - Port Operations

- (void)openPort:(NSString *)portId
         resolve:(RCTPromiseResolveBlock)resolve
          reject:(RCTPromiseRejectBlock)reject {
    if (!_portManager) {
        reject(@"MIDI_ERROR", @"MIDI not initialized. Call requestMIDIAccess first.", nil);
        return;
    }

    NSError *error = nil;
    BOOL success = [_portManager openPort:portId error:&error];

    if (success) {
        resolve(@YES);
    } else {
        reject(@"MIDI_ERROR", error.localizedDescription, error);
    }
}

- (void)closePort:(NSString *)portId
          resolve:(RCTPromiseResolveBlock)resolve
           reject:(RCTPromiseRejectBlock)reject {
    if (!_portManager) {
        reject(@"MIDI_ERROR", @"MIDI not initialized. Call requestMIDIAccess first.", nil);
        return;
    }

    NSError *error = nil;
    BOOL success = [_portManager closePort:portId error:&error];

    if (success) {
        resolve(@YES);
    } else {
        reject(@"MIDI_ERROR", error.localizedDescription, error);
    }
}

#pragma mark - Message Operations

- (void)sendMIDIMessage:(NSString *)portId
                   data:(NSArray<NSNumber *> *)data
              timestamp:(NSNumber *)timestamp {
    if (!_portManager) {
        NSLog(@"MIDI not initialized");
        return;
    }

    NSError *error = nil;
    BOOL success = [_portManager sendMessage:data toPort:portId timestamp:timestamp error:&error];

    if (!success) {
        NSLog(@"Failed to send MIDI message: %@", error.localizedDescription);
    }
}

- (void)clearPendingMessages:(NSString *)portId {
    if (!_portManager) {
        return;
    }

    [_portManager clearPendingMessages:portId];
}

#pragma mark - Event Listeners

- (void)addListener:(NSString *)portId {
    [self.eventEmitter startObservingPortId:portId];
}

- (void)removeListener:(NSString *)portId {
    [self.eventEmitter stopObservingPortId:portId];
}

#pragma mark - TurboModule

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params {
    return std::make_shared<facebook::react::NativeMediSpecJSI>(params);
}

+ (NSString *)moduleName {
    return @"Medi";
}

@end
