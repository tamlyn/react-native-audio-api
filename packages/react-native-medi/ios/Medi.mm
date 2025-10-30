#import "Medi.h"

@implementation Medi

RCT_EXPORT_MODULE()

- (instancetype)init {
  self = [super init];
  if (self) {
    self.midiClient = 0;
    self.openPorts = [NSMutableDictionary dictionary];
  }
  return self;
}

static void midiNotifyProc(const MIDINotification *message) {
  // TODO: for now these are just logged and are not sent to JS
  switch (message->messageID) {
    case kMIDIMsgSetupChanged: NSLog(@"[MIDI Notify] Setup changed"); break;
    case kMIDIMsgObjectAdded: NSLog(@"[MIDI Notify] Object added"); break;
    case kMIDIMsgObjectRemoved: NSLog(@"[MIDI Notify] Object removed"); break;
    case kMIDIMsgPropertyChanged: NSLog(@"[MIDI Notify] Property changed"); break;
    case kMIDIMsgThruConnectionsChanged: NSLog(@"[MIDI Notify] Thru connections changed"); break;
    case kMIDIMsgSerialPortOwnerChanged: NSLog(@"[MIDI Notify] Serial port owner changed"); break;
    case kMIDIMsgIOError: NSLog(@"[MIDI Notify] I/O error"); break;
    default: NSLog(@"[MIDI Notify] Message %u", (unsigned)message->messageID); break;
  }
}

// Instance method version so that `self` is available inside the callback.
- (void)midiEvent:(const MIDIEventList *)eventList srcConnRefCon:(void *)srcConnRefCon {
  MIDIEndpointRef endpoint = (MIDIEndpointRef)(uintptr_t)srcConnRefCon;

  // Cached port and timestamp for performance
  SInt32 uniqueID = 0;
  MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &uniqueID);
  NSString *portId = [NSString stringWithFormat:@"input_%d", (int)uniqueID];
  NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970] * 1000;  // timestamp in milliseconds

  const MIDIEventPacket *packet = &eventList->packet[0];
  for (UInt32 i = 0; i < eventList->numPackets; i++) {
    UInt32 *words = (UInt32 *)&packet->words[0];
    UInt8 status = words[0] >> 16;
    UInt8 data1 = words[0] >> 8;
    UInt8 data2 = words[0];

    // Emit event to JS (generated emitter from the spec)
    [self emitOnMidiMessage:@{
      @"portId": portId,
      @"data": @[@(status), @(data1), @(data2)],
      @"timestamp": @(timestamp)
    }];
    packet = MIDIEventPacketNext(packet);
  }
}

RCT_EXPORT_METHOD(prepareMIDIClient:(BOOL)sysex
                  resolve:(RCTPromiseResolveBlock)resolve
                  reject:(RCTPromiseRejectBlock)reject)
{
  if (self.midiClient != 0) {
    resolve(nil);
    return;
  }

  /// IMPORTANT: CoreMIDI is shit as hell
  /// consecutive calls to MIDIGetNumberOfSources() does not reflect new devices correctly because under the hood
  /// it shedules some updates asynchronously and JS thread does not use CFRunLoop (which is insane pattern but welcome to objective-c world)
  /// It is nowhere mentioned in the docs and i still find it weird so you need to trust me on this one
  /// Calling MIDIClientCreateWithBlock on main thread seems to fix this issue, client connects to main thread run loop queue
  /// But the one problem i do not want to deal with more is why MIDIGetNumberOfSources() does work correctly only after this call
  /// It seems like they share same run loop update mechanism and some internal global state which is wild because who designs such APIs
  /// without any documentation about it
  ///
  /// Hours wasted: ~16
  ///
  /// PS: Not actually wasted time cuz it works but brooo☠
  dispatch_async(dispatch_get_main_queue(), ^{
    OSStatus status = MIDIClientCreateWithBlock(CFSTR("MediClient"), &_midiClient, ^(const MIDINotification *message) {
      midiNotifyProc(message);
    });

    if (status != noErr) {
      reject(@"MIDI_ERROR", [NSString stringWithFormat:@"Failed to create MIDI client: %d", status], nil);
      return;
    }

    self.sysexEnabled = sysex;
    NSLog(@"MIDIClientCreateWithBlock status: %d, sysex: %d", status, sysex);
    OSStatus statusPort = MIDIInputPortCreateWithProtocol(self.midiClient, CFSTR("MediInputPort"), kMIDIProtocol_1_0, &_inputPort, ^(const MIDIEventList *eventList, void *srcConnRefCon) {
      [self midiEvent:eventList srcConnRefCon:srcConnRefCon];
    });

    if (statusPort != noErr) {
      reject(@"MIDI_ERROR", [NSString stringWithFormat:@"Failed to create MIDI input port: %d", statusPort], nil);
      return;
    }

    NSLog(@"MIDIInputPortCreateWithProtocol status: %d", statusPort);

    // Resolve the promise after MIDI client is fully initialized
    resolve(nil);
  });
}

RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSArray *, getSources)
{
  ItemCount sourceCount = MIDIGetNumberOfSources();
  NSMutableArray *inputs = [NSMutableArray array];
  for (ItemCount i = 0; i < sourceCount; ++i) {
    MIDIEndpointRef src = MIDIGetSource(i);
    if (src) {
      NSDictionary *info = [self getEndpointInfo:src type:@"input"];
      [inputs addObject:info];
    }
  }
  NSLog(@"MIDI Sources: %@", inputs);
  return inputs;
}

RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSArray *, getDestinations)
{
  ItemCount destCount = MIDIGetNumberOfDestinations();
  NSMutableArray *outputs = [NSMutableArray array];
  for (ItemCount i = 0; i < destCount; ++i) {
    MIDIEndpointRef dest = MIDIGetDestination(i);
    if (dest) {
      NSDictionary *info = [self getEndpointInfo:dest type:@"output"];
      [outputs addObject:info];
    }
  }
  NSLog(@"MIDI Destinations: %@", outputs);
  return outputs;
}


RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSNumber *, openPort:(NSString *)portId)
{
  NSString *type = [portId hasPrefix:@"input_"] ? @"input" : @"output";
  SInt32 uniqueID = [[portId substringFromIndex:[type length] + 1] intValue];
  if ([type isEqualToString:@"output"]) {
    // We do not need to open output ports so it is considered always successful
    NSLog(@"Opened output port: %@", portId);
    return @YES;
  }
  MIDIEndpointRef endpoint = 0;
  ItemCount srcCount = MIDIGetNumberOfSources();
  for (ItemCount i = 0; i < srcCount; ++i) {
    MIDIEndpointRef src = MIDIGetSource(i);
    SInt32 srcUniqueID = 0;
    MIDIObjectGetIntegerProperty(src, kMIDIPropertyUniqueID, &srcUniqueID);
    if (srcUniqueID == uniqueID) {
      endpoint = src;
      break;
    }
  }
  if (endpoint == 0) {
    NSLog(@"Port not found: %@", portId);
    return @NO;
  }

  OSStatus status = MIDIPortConnectSource(self.inputPort, endpoint, (void *)(uintptr_t)endpoint);
  if (status != noErr) {
    NSLog(@"Failed to connect input port: %@, status: %d", portId, status);
    return @NO;
  }

  self.openPorts[portId] = @(endpoint);
  NSLog(@"Opened port: %@", portId);
  return @YES;
}

RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSNumber *, closePort:(NSString *)portId)
{
  NSString *type = [portId hasPrefix:@"input_"] ? @"input" : @"output";
  if (!self.openPorts[portId]) {
    return @NO;
  }
  MIDIEndpointRef endpoint = [self.openPorts[portId] unsignedIntValue];
  if ([type isEqualToString:@"input"]) {
    MIDIPortDisconnectSource(self.inputPort, endpoint);
  }
  [self.openPorts removeObjectForKey:portId];
  return @YES;
}

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

    /// CoreMIDI is shit as hell
    /// It uses old reference counting and has a lot of quirks
    /// __bridge_transfer here just transfers ownership from CoreFoundation to ARC
    /// without it the memory would get deallocated too early
    NSDictionary *info = @{
        @"id": portId,
        @"name": name ? (__bridge_transfer NSString *)name : [NSNull null],
        @"manufacturer": manufacturer ? (__bridge_transfer NSString *)manufacturer : [NSNull null],
        @"type": type,
        @"version": [NSNull null],
        @"state": isActive ? @"connected" : @"disconnected",
        @"connection": self.openPorts[portId] ? @"open" : @"closed"
    };

    return info;
}

#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<facebook::react::NativeMediSpecJSI>(params);
}
#endif

@end
