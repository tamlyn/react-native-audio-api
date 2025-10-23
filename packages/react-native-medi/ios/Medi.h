#ifdef RCT_NEW_ARCH_ENABLED
#import <MediSpec/MediSpec.h>
#endif

#import <React/RCTBridgeModule.h>
#import <CoreMIDI/CoreMIDI.h>


@interface Medi : NSObject
#ifdef RCT_NEW_ARCH_ENABLED
                   <NativeMediSpec>
#else
                   <RCTBridgeModule>
#endif
{
  MIDIClientRef midiClient;
  BOOL sysexEnabled;
  MIDIPortRef inputPort;
  MIDIPortRef outputPort;
}

@property (nonatomic) MIDIClientRef midiClient;
@property (nonatomic) MIDIPortRef inputPort;
@property (nonatomic) MIDIPortRef outputPort;
@property (nonatomic) BOOL sysexEnabled;
@property (nonatomic, strong) NSMutableDictionary* openPorts;

@end
