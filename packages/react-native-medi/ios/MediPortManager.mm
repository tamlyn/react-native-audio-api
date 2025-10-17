#import "MediPortManager.h"
#import "MediEventEmitter.h"

// Forward declaration of context structure from Medi.mm
typedef struct {
    void *portId;  // retained NSString
    void *emitter; // unretained MediEventEmitter
} MIDIReadContext;

@implementation MediPortManager {
    NSMutableDictionary *_openPorts;
    NSMutableDictionary *_portIdReferences; // Keep strong references to portId strings
}

- (instancetype)initWithClient:(MIDIClientRef)client
                     inputPort:(MIDIPortRef)inputPort
                    outputPort:(MIDIPortRef)outputPort
                  eventEmitter:(MediEventEmitter *)eventEmitter {
    if (self = [super init]) {
        _midiClient = client;
        _midiInputPort = inputPort;
        _midiOutputPort = outputPort;
        _eventEmitter = eventEmitter;
        _openPorts = [NSMutableDictionary dictionary];
        _portIdReferences = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)dealloc {
    // Release all retained portId references and free contexts
    [_portIdReferences enumerateKeysAndObjectsUsingBlock:^(id key, NSValue *contextValue, BOOL *stop) {
        MIDIReadContext *context = (MIDIReadContext *)[contextValue pointerValue];
        if (context) {
            CFBridgingRelease(context->portId);
            free(context);
        }
    }];
}

#pragma mark - Device Enumeration

- (NSDictionary *)getEndpointInfo:(MIDIEndpointRef)endpoint type:(NSString *)type {
    CFStringRef name = NULL;
    CFStringRef manufacturer = NULL;
    SInt32 uniqueID = 0;
    SInt32 isOffline = 0;

    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &name);
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyManufacturer, &manufacturer);
    MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &uniqueID);
    MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyOffline, &isOffline);

    NSString *portId = [NSString stringWithFormat:@"%@_%d", type, (int)uniqueID];
    BOOL isActive = (isOffline == 0);

    NSDictionary *info = @{
        @"id": portId,
        @"name": name ? (__bridge_transfer NSString *)name : [NSNull null],
        @"manufacturer": manufacturer ? (__bridge_transfer NSString *)manufacturer : [NSNull null],
        @"type": type,
        @"version": [NSNull null],
        @"state": isActive ? @"connected" : @"disconnected",
        @"connection": _openPorts[portId] ? @"open" : @"closed"
    };

    return info;
}

- (NSArray<NSDictionary *> *)getInputPorts {
    NSMutableArray *inputs = [NSMutableArray array];
    ItemCount sourceCount = MIDIGetNumberOfSources();

    for (ItemCount i = 0; i < sourceCount; i++) {
        MIDIEndpointRef source = MIDIGetSource(i);
        [inputs addObject:[self getEndpointInfo:source type:@"input"]];
    }

    return inputs;
}

- (NSArray<NSDictionary *> *)getOutputPorts {
    NSMutableArray *outputs = [NSMutableArray array];
    ItemCount destCount = MIDIGetNumberOfDestinations();

    for (ItemCount i = 0; i < destCount; i++) {
        MIDIEndpointRef destination = MIDIGetDestination(i);
        [outputs addObject:[self getEndpointInfo:destination type:@"output"]];
    }

    return outputs;
}

#pragma mark - Port Operations

- (MIDIEndpointRef)getEndpointForPortId:(NSString *)portId isInput:(BOOL *)isInput {
    NSArray *components = [portId componentsSeparatedByString:@"_"];
    if (components.count != 2) return 0;

    NSString *type = components[0];
    SInt32 uniqueID = [components[1] intValue];

    *isInput = [type isEqualToString:@"input"];

    ItemCount count = *isInput ? MIDIGetNumberOfSources() : MIDIGetNumberOfDestinations();

    for (ItemCount i = 0; i < count; i++) {
        MIDIEndpointRef endpoint = *isInput ? MIDIGetSource(i) : MIDIGetDestination(i);
        SInt32 endpointID;
        MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &endpointID);

        if (endpointID == uniqueID) {
            return endpoint;
        }
    }

    return 0;
}

- (BOOL)openPort:(NSString *)portId error:(NSError **)error {
    if (_openPorts[portId]) {
        return YES;
    }

    BOOL isInput;
    MIDIEndpointRef endpoint = [self getEndpointForPortId:portId isInput:&isInput];

    if (!endpoint) {
        if (error) {
            *error = [NSError errorWithDomain:@"MediError"
                                         code:404
                                     userInfo:@{NSLocalizedDescriptionKey: @"Port not found"}];
        }
        return NO;
    }

    if (isInput) {
        // For input ports, connect to receive messages
        // Create context structure with portId and emitter
        NSString *portIdCopy = [portId copy];
        void *portIdRef = (void *)CFBridgingRetain(portIdCopy);

        // Allocate and populate context
        MIDIReadContext *context = (MIDIReadContext *)malloc(sizeof(MIDIReadContext));
        context->portId = portIdRef;
        context->emitter = (__bridge void *)_eventEmitter;  // unretained reference

        // Store the context so we can free it later
        _portIdReferences[portId] = [NSValue valueWithPointer:context];

        OSStatus status = MIDIPortConnectSource(_midiInputPort, endpoint, context);
        if (status != noErr) {
            // Clean up on failure
            CFBridgingRelease(portIdRef);
            free(context);
            [_portIdReferences removeObjectForKey:portId];

            if (error) {
                *error = [NSError errorWithDomain:@"MediError"
                                             code:status
                                         userInfo:@{NSLocalizedDescriptionKey: @"Failed to connect to MIDI source"}];
            }
            return NO;
        }
    }

    _openPorts[portId] = @(endpoint);

    // Send state change event
    if (_eventEmitter) {
        [_eventEmitter sendStateChangeEvent:portId
                                      state:@"connected"
                                 connection:@"open"];
    }

    return YES;
}

- (BOOL)closePort:(NSString *)portId error:(NSError **)error {
    if (!_openPorts[portId]) {
        return YES;
    }

    BOOL isInput;
    MIDIEndpointRef endpoint = [self getEndpointForPortId:portId isInput:&isInput];

    if (endpoint && isInput) {
        MIDIPortDisconnectSource(_midiInputPort, endpoint);

        // Free the context and release the retained portId reference
        NSValue *contextValue = _portIdReferences[portId];
        if (contextValue) {
            MIDIReadContext *context = (MIDIReadContext *)[contextValue pointerValue];
            if (context) {
                CFBridgingRelease(context->portId);
                free(context);
            }
            [_portIdReferences removeObjectForKey:portId];
        }
    }

    [_openPorts removeObjectForKey:portId];

    // Send state change event
    if (_eventEmitter) {
        [_eventEmitter sendStateChangeEvent:portId
                                          state:@"connected"
                                     connection:@"closed"];
    }

    return YES;
}

- (BOOL)isPortOpen:(NSString *)portId {
    return _openPorts[portId] != nil;
}

#pragma mark - Message Operations

- (BOOL)sendMessage:(NSArray<NSNumber *> *)data
             toPort:(NSString *)portId
          timestamp:(NSNumber *)timestamp
              error:(NSError **)error {
    if (!_openPorts[portId]) {
        if (error) {
            *error = [NSError errorWithDomain:@"MediError"
                                         code:400
                                     userInfo:@{NSLocalizedDescriptionKey: @"Port is not open"}];
        }
        return NO;
    }

    BOOL isInput;
    MIDIEndpointRef endpoint = [self getEndpointForPortId:portId isInput:&isInput];

    if (!endpoint || isInput) {
        if (error) {
            *error = [NSError errorWithDomain:@"MediError"
                                         code:400
                                     userInfo:@{NSLocalizedDescriptionKey: @"Invalid output port"}];
        }
        return NO;
    }

    // Convert data array to bytes
    Byte buffer[data.count];
    for (NSUInteger i = 0; i < data.count; i++) {
        buffer[i] = [data[i] unsignedCharValue];
    }

    // Create MIDI packet
    Byte packetBuffer[1024];
    MIDIPacketList *packetList = (MIDIPacketList *)packetBuffer;
    MIDIPacket *packet = MIDIPacketListInit(packetList);

    MIDITimeStamp midiTimestamp = timestamp ? (MIDITimeStamp)([timestamp doubleValue] * 1000000.0) : 0;
    packet = MIDIPacketListAdd(packetList, sizeof(packetBuffer), packet, midiTimestamp, data.count, buffer);

    if (!packet) {
        if (error) {
            *error = [NSError errorWithDomain:@"MediError"
                                         code:500
                                     userInfo:@{NSLocalizedDescriptionKey: @"Failed to create MIDI packet"}];
        }
        return NO;
    }

    OSStatus status = MIDISend(_midiOutputPort, endpoint, packetList);
    if (status != noErr) {
        if (error) {
            *error = [NSError errorWithDomain:@"MediError"
                                         code:status
                                     userInfo:@{NSLocalizedDescriptionKey: [NSString stringWithFormat:@"Failed to send MIDI message: %d", (int)status]}];
        }
        return NO;
    }

    return YES;
}

- (void)clearPendingMessages:(NSString *)portId {
    BOOL isInput;
    MIDIEndpointRef endpoint = [self getEndpointForPortId:portId isInput:&isInput];

    if (endpoint && !isInput) {
        MIDIFlushOutput(endpoint);
    }
}

@end
