#import "Medi.h"

@implementation Medi

RCT_EXPORT_MODULE()

- (instancetype)init {
  self = [super init];
  if (self) {
    // Initialization code if needed
  }
  return self;
}
static void midiNotifyProc(const MIDINotification *message, void *refCon) {
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

RCT_EXPORT_METHOD(test)
{
  if (midiClient != 0) {
    ItemCount sourceCount = MIDIGetNumberOfSources();
    NSLog(@"Number of MIDI sources: %u", (unsigned)sourceCount);
    return;
  }
  dispatch_async(dispatch_get_main_queue(), ^{
    OSStatus status = MIDIClientCreateWithBlock(CFSTR("MediClient"), &midiClient, ^(const MIDINotification *message) {
      midiNotifyProc(message, NULL);
    });
    NSLog(@"MIDIClientCreateWithBlock status: %d", status);
  });
}

RCT_EXPORT_METHOD(prepareMIDIClient)
{
  if (midiClient != 0) {
    return;
  }
  dispatch_async(dispatch_get_main_queue(), ^{
    OSStatus status = MIDIClientCreateWithBlock(CFSTR("MediClient"), &midiClient, ^(const MIDINotification *message) {
      midiNotifyProc(message, NULL);
    });
    NSLog(@"MIDIClientCreateWithBlock status: %d", status);
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
        @"connection": @"closed" // TODO change in future when we manage connections
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
