#import <Foundation/Foundation.h>
#import <CoreMIDI/CoreMIDI.h>

@class MediEventEmitter;

@interface MediPortManager : NSObject

@property (nonatomic, assign) MIDIClientRef midiClient;
@property (nonatomic, assign) MIDIPortRef midiInputPort;
@property (nonatomic, assign) MIDIPortRef midiOutputPort;
@property (nonatomic, weak) MediEventEmitter *eventEmitter;

- (instancetype)initWithClient:(MIDIClientRef)client
                     inputPort:(MIDIPortRef)inputPort
                    outputPort:(MIDIPortRef)outputPort
                  eventEmitter:(MediEventEmitter *)eventEmitter;

// Device enumeration
- (NSDictionary *)getEndpointInfo:(MIDIEndpointRef)endpoint type:(NSString *)type;
- (NSArray<NSDictionary *> *)getInputPorts;
- (NSArray<NSDictionary *> *)getOutputPorts;

// Port operations
- (MIDIEndpointRef)getEndpointForPortId:(NSString *)portId isInput:(BOOL *)isInput;
- (BOOL)openPort:(NSString *)portId error:(NSError **)error;
- (BOOL)closePort:(NSString *)portId error:(NSError **)error;
- (BOOL)isPortOpen:(NSString *)portId;

// Message operations
- (BOOL)sendMessage:(NSArray<NSNumber *> *)data
             toPort:(NSString *)portId
          timestamp:(NSNumber *)timestamp
              error:(NSError **)error;
- (void)clearPendingMessages:(NSString *)portId;

@end
