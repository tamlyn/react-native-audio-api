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

static void midiEvent(const MIDIEventList *eventList, void *srcConnRefCon) {
    MIDIEndpointRef endpoint = (MIDIEndpointRef)(uintptr_t)srcConnRefCon;

    SInt32 uniqueID = 0;
    MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &uniqueID);
    NSString *portId = [NSString stringWithFormat:@"input_%d", (int)uniqueID];

    const MIDIEventPacket *packet = &eventList->packet[0];
    for (UInt32 i = 0; i < eventList->numPackets; i++) {
        UInt32 *words = (UInt32 *)&packet->words[0];
        UInt8 status = (words[0] >> 16) & 0xFF;
        UInt8 data1 = (words[0] >> 8) & 0xFF;
        UInt8 data2 = words[0] & 0xFF;

        // TODO: send these events to JS side
        // for now just loging
        NSLog(@"Port: %@, Status: 0x%02X, Data1: 0x%02X, Data2: 0x%02X", portId, status, data1, data2);

        packet = MIDIEventPacketNext(packet);
    }
}

RCT_EXPORT_METHOD(test)
{
  if (self.midiClient != 0) {
    ItemCount sourceCount = MIDIGetNumberOfSources();
    NSLog(@"Number of MIDI sources: %u", (unsigned)sourceCount);
    return;
  }
  dispatch_async(dispatch_get_main_queue(), ^{
    OSStatus status = MIDIClientCreateWithBlock(CFSTR("MediClient"), &_midiClient, ^(const MIDINotification *message) {
      midiNotifyProc(message);
    });
    NSLog(@"MIDIClientCreateWithBlock status: %d", status);
  });
}

RCT_EXPORT_METHOD(prepareMIDIClient:(BOOL)sysex)
{
  if (self.midiClient != 0) {
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
  dispatch_async(dispatch_get_main_queue(), ^{
    OSStatus status = MIDIClientCreateWithBlock(CFSTR("MediClient"), &_midiClient, ^(const MIDINotification *message) {
      midiNotifyProc(message);
    });
    self.sysexEnabled = sysex;
    NSLog(@"MIDIClientCreateWithBlock status: %d, sysex: %d", status, sysex);
    OSStatus statusPort = MIDIInputPortCreateWithProtocol(self.midiClient, CFSTR("MediInputPort"), kMIDIProtocol_1_0, &_inputPort, ^(const MIDIEventList *eventList, void *srcConnRefCon) {
      midiEvent(eventList, srcConnRefCon);
    });
    NSLog(@"MIDIInputPortCreateWithProtocol status: %d", statusPort);

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
  MIDIEndpointRef endpoint = 0;
  if ([type isEqualToString:@"input"]) {
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
  } else {
    ItemCount destCount = MIDIGetNumberOfDestinations();
    for (ItemCount i = 0; i < destCount; ++i) {
      MIDIEndpointRef dest = MIDIGetDestination(i);
      SInt32 destUniqueID = 0;
      MIDIObjectGetIntegerProperty(dest, kMIDIPropertyUniqueID, &destUniqueID);
      if (destUniqueID == uniqueID) {
        endpoint = dest;
        break;
      }
    }
  }
  if (endpoint == 0) {
    NSLog(@"Port not found: %@", portId);
    return @NO;
  }

  if ([type isEqualToString:@"input"]) {
    OSStatus status = MIDIPortConnectSource(self.inputPort, endpoint, (void *)(uintptr_t)endpoint);
    if (status != noErr) {
      NSLog(@"Failed to connect input port: %@, status: %d", portId, status);
      return @NO;
    }
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
