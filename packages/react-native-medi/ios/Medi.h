#import <MediSpec/MediSpec.h>
#import <CoreMIDI/CoreMIDI.h>


@interface Medi : NativeMediSpecBase<NativeMediSpec>

@property (nonatomic) MIDIClientRef midiClient;
@property (nonatomic) MIDIPortRef inputPort;
@property (nonatomic) MIDIPortRef outputPort;
@property (nonatomic) BOOL sysexEnabled;
@property (nonatomic, strong) NSMutableDictionary* openPorts;

@end
