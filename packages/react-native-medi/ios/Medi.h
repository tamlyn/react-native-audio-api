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

}

@end
