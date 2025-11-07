#pragma once

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>

@interface AudioSessionManager : NSObject

@property (nonatomic, weak) AVAudioSession *audioSession;

// State tracking
@property (nonatomic, assign) bool isActive;
@property (nonatomic, assign) bool shouldManageSession;

// Session configuration options (desired by user)
@property (nonatomic, assign) AVAudioSessionMode desiredMode;
@property (nonatomic, assign) AVAudioSessionCategory desiredCategory;
@property (nonatomic, assign) AVAudioSessionCategoryOptions desiredOptions;
@property (nonatomic, assign) bool allowHapticsAndSounds;

- (instancetype)init;
- (void)cleanup;

- (void)disableSessionManagement;
- (void)setAudioSessionOptions:(NSString *)category
                          mode:(NSString *)mode
                       options:(NSArray *)options
                  allowHaptics:(BOOL)allowHaptics;

- (bool)setActive:(bool)active;
- (void)markInactive;
- (void)disableSessionManagement;

- (NSNumber *)getDevicePreferredSampleRate;

- (void)requestRecordingPermissions:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject;
- (void)checkRecordingPermissions:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject;

- (void)getDevicesInfo:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject;
- (NSArray<NSDictionary *> *)parseDeviceList:(NSArray<AVAudioSessionPortDescription *> *)devices;

@end
