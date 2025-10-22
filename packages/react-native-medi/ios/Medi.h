#import <MediSpec/MediSpec.h>
#import <React/RCTBridgeModule.h>
#import <CoreMIDI/CoreMIDI.h>

@class RCTBridge;

@interface Medi : NSObject <NativeMediSpec, RCTBridgeModule>
- (void) test;
@end
