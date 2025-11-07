#import <audioapi/ios/system/AudioSessionManager.h>

@implementation AudioSessionManager

- (instancetype)init
{
  if (self = [super init]) {
    self.audioSession = [AVAudioSession sharedInstance];

    self.isActive = false;
    self.shouldManageSession = true;

    self.desiredCategory = AVAudioSessionCategoryPlayback;
    self.desiredMode = AVAudioSessionModeDefault;
    self.desiredOptions = 0;
    self.allowHapticsAndSounds = false;
  }

  return self;
}

- (void)cleanup
{
  self.audioSession = nil;
}

- (bool)areDesiredOptionsSet
{
  return (
      self.audioSession.category == self.desiredCategory && self.audioSession.mode == self.desiredMode &&
      self.audioSession.categoryOptions == self.desiredOptions &&
      self.audioSession.allowHapticsAndSystemSoundsDuringRecording == self.allowHapticsAndSounds);
}

- (bool)configureAudioSession
{
  NSError *error = nil;

  if (!self.shouldManageSession || [self areDesiredOptionsSet]) {
    return true;
  }

  [self.audioSession setCategory:self.desiredCategory mode:self.desiredMode options:self.desiredOptions error:&error];

  if (error != nil) {
    NSLog(@"Error while configuring audio session: %@", [error debugDescription]);
    return false;
  }

  if (self.audioSession.allowHapticsAndSystemSoundsDuringRecording != self.allowHapticsAndSounds) {
    [self.audioSession setAllowHapticsAndSystemSoundsDuringRecording:self.allowHapticsAndSounds error:&error];

    if (error != nil) {
      NSLog(@"Error while setting allowHapticsAndSystemSoundsDuringRecording: %@", [error debugDescription]);
      return false;
    }
  }

  return true;
}

- (void)setAudioSessionOptions:(NSString *)category
                          mode:(NSString *)mode
                       options:(NSArray *)options
                  allowHaptics:(BOOL)allowHaptics
{
  AVAudioSessionCategory category = [self categoryFromString:categoryStr];
  AVAudioSessionMode mode = [self modeFromString:modeStr];
  AVAudioSessionCategoryOptions options = [self optionsFromArray:optionsArray];
  bool configChanged = false;

  if (category != self.desiredCategory || mode != self.desiredMode || options != self.desiredOptions ||
      allowHaptics != self.allowHapticsAndSounds) {
    configChanged = true;
  }

  self.desiredCategory = category;
  self.desiredMode = mode;
  self.desiredOptions = options;
  self.allowHapticsAndSounds = allowHaptics;

  if (configChanged && self.isActive) {
    [self configureAudioSession];
  }
}

- (bool)setActive:(bool)active
{
  if (!self.shouldManageSession) {
    return true;
  }

  NSError *error = nil;
  bool success = false;

  if (self.isActive == active) {
    return true;
  }

  if (active) {
    success = [self configureAudioSession];

    if (!success) {
      return false;
    }
  }

  success = [self.audioSession setActive:active error:&error];

  if (success) {
    self.isActive = active;
  }

  if (error != nil) {
    NSLog(@"[AudioSessionManager] setting session as %@ failed", active ? @"ACTIVE" : @"INACTIVE");
  } else {
    NSLog(@"[AudioSessionManager] session is %@", active ? @"ACTIVE" : @"INACTIVE");
  }

  return success;
}

- (void)markInactive
{
  // Mark as inactive no matter the state reported by AVAudioSession,
  // this is used during interruptions to "force" going through configure&activate flow
  // which is necessary after some of the interruptions (f.e. when the other app re-configures the hardware)
  self.isActive = false;
}

- (void)disableSessionManagement
{
  self.shouldManageSession = false;
  self.hasDirtySettings = false;
}

- (NSNumber *)getDevicePreferredSampleRate
{
  return [NSNumber numberWithFloat:[self.audioSession sampleRate]];
}

- (void)requestRecordingPermissions:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject
{
  id value = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSMicrophoneUsageDescription"];
  // if there is no entry NSMicrophoneUsageDescription calling requestRecordPermission will quit an app
  if (value == nil) {
    reject(
        nil,
        @"There is no NSMicrophoneUsageDescription entry in info.plist file. App cannot access microphone without it.",
        nil);
    return;
  }
  if (@available(iOS 17, *)) {
    [AVAudioSession.sharedInstance requestRecordPermission:^(BOOL granted) {
      if (granted) {
        resolve(@"Granted");
      } else {
        resolve(@"Denied");
      }
    }];
  } else {
    [self.audioSession requestRecordPermission:^(BOOL granted) {
      if (granted) {
        resolve(@"Granted");
      } else {
        resolve(@"Denied");
      }
    }];
  }
}

- (void)checkRecordingPermissions:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject
{
  if (@available(iOS 17, *)) {
    NSInteger res = [[AVAudioApplication sharedInstance] recordPermission];
    switch (res) {
      case AVAudioApplicationRecordPermissionUndetermined:
        resolve(@"Undetermined");
        break;
      case AVAudioApplicationRecordPermissionGranted:
        resolve(@"Granted");
        break;
      case AVAudioApplicationRecordPermissionDenied:
        resolve(@"Denied");
        break;
      default:
        resolve(@"Undetermined");
        break;
    }
  } else {
    NSInteger res = [self.audioSession recordPermission];
    switch (res) {
      case AVAudioSessionRecordPermissionUndetermined:
        resolve(@"Undetermined");
        break;
      case AVAudioSessionRecordPermissionGranted:
        resolve(@"Granted");
        break;
      case AVAudioSessionRecordPermissionDenied:
        resolve(@"Denied");
        break;
      default:
        resolve(@"Undetermined");
        break;
    }
  }
}

- (NSString *)checkRecordingPermissions
{
  if (@available(iOS 17, *)) {
    NSInteger res = [[AVAudioApplication sharedInstance] recordPermission];
    switch (res) {
      case AVAudioApplicationRecordPermissionUndetermined:
        return @"Undetermined";
      case AVAudioApplicationRecordPermissionGranted:
        return @"Granted";
      case AVAudioApplicationRecordPermissionDenied:
        return @"Denied";
      default:
        return @"Undetermined";
    }
  } else {
    NSInteger res = [self.audioSession recordPermission];
    switch (res) {
      case AVAudioSessionRecordPermissionUndetermined:
        return @"Undetermined";
      case AVAudioSessionRecordPermissionGranted:
        return @"Granted";
      case AVAudioSessionRecordPermissionDenied:
        return @"Denied";
      default:
        return @"Undetermined";
    }
  }
}

- (void)getDevicesInfo:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject
{
  NSMutableDictionary *devicesInfo = [[NSMutableDictionary alloc] init];

  [devicesInfo setValue:[self parseDeviceList:[self.audioSession availableInputs]] forKey:@"availableInputs"];
  [devicesInfo setValue:[self parseDeviceList:[[self.audioSession currentRoute] inputs]] forKey:@"currentInputs"];
  [devicesInfo setValue:[self parseDeviceList:[[self.audioSession currentRoute] outputs]] forKey:@"availableOutputs"];
  [devicesInfo setValue:[self parseDeviceList:[[self.audioSession currentRoute] outputs]] forKey:@"currentOutputs"];

  resolve(devicesInfo);
}

- (NSArray<NSDictionary *> *)parseDeviceList:(NSArray<AVAudioSessionPortDescription *> *)devices
{
  NSMutableArray<NSDictionary *> *deviceList = [[NSMutableArray alloc] init];

  for (AVAudioSessionPortDescription *device in devices) {
    [deviceList addObject:@{
      @"name" : device.portName,
      @"category" : device.portType,
    }];
  }

  return deviceList;
}

- (AVAudioSessionCategory)categoryFromString:(NSString *)categorySTR
{
  AVAudioSessionCategory category = 0;

  if ([categorySTR isEqualToString:@"record"]) {
    category = AVAudioSessionCategoryRecord;
  } else if ([categorySTR isEqualToString:@"ambient"]) {
    category = AVAudioSessionCategoryAmbient;
  } else if ([categorySTR isEqualToString:@"playback"]) {
    category = AVAudioSessionCategoryPlayback;
  } else if ([categorySTR isEqualToString:@"multiRoute"]) {
    category = AVAudioSessionCategoryMultiRoute;
  } else if ([categorySTR isEqualToString:@"soloAmbient"]) {
    category = AVAudioSessionCategorySoloAmbient;
  } else if ([categorySTR isEqualToString:@"playAndRecord"]) {
    category = AVAudioSessionCategoryPlayAndRecord;
  }

  return category;
}

- (AVAudioSessionMode)modeFromString:(NSString *)modeSTR
{
  AVAudioSessionMode mode = 0;

  if ([modeSTR isEqualToString:@"default"]) {
    mode = AVAudioSessionModeDefault;
  } else if ([modeSTR isEqualToString:@"gameChat"]) {
    mode = AVAudioSessionModeGameChat;
  } else if ([modeSTR isEqualToString:@"videoChat"]) {
    mode = AVAudioSessionModeVideoChat;
  } else if ([modeSTR isEqualToString:@"voiceChat"]) {
    mode = AVAudioSessionModeVoiceChat;
  } else if ([modeSTR isEqualToString:@"measurement"]) {
    mode = AVAudioSessionModeMeasurement;
  } else if ([modeSTR isEqualToString:@"voicePrompt"]) {
    mode = AVAudioSessionModeVoicePrompt;
  } else if ([modeSTR isEqualToString:@"spokenAudio"]) {
    mode = AVAudioSessionModeSpokenAudio;
  } else if ([modeSTR isEqualToString:@"moviePlayback"]) {
    mode = AVAudioSessionModeMoviePlayback;
  } else if ([modeSTR isEqualToString:@"videoRecording"]) {
    mode = AVAudioSessionModeVideoRecording;
  }

  return mode;
}

- (AVAudioSessionCategoryOptions)optionsFromArray:(NSArray *)optionsArray
{
  AVAudioSessionCategoryOptions options = 0;

  for (NSString *option in optionsArray) {
    if ([option isEqualToString:@"duckOthers"]) {
      options |= AVAudioSessionCategoryOptionDuckOthers;
    }

    if ([option isEqualToString:@"allowAirPlay"]) {
      options |= AVAudioSessionCategoryOptionAllowAirPlay;
    }

    if ([option isEqualToString:@"mixWithOthers"]) {
      options |= AVAudioSessionCategoryOptionMixWithOthers;
    }

    if ([option isEqualToString:@"allowBluetooth"]) {
      options |= AVAudioSessionCategoryOptionAllowBluetooth;
    }

    if ([option isEqualToString:@"defaultToSpeaker"]) {
      options |= AVAudioSessionCategoryOptionDefaultToSpeaker;
    }

    if ([option isEqualToString:@"allowBluetoothA2DP"]) {
      options |= AVAudioSessionCategoryOptionAllowBluetoothA2DP;
    }

    if ([option isEqualToString:@"overrideMutedMicrophoneInterruption"]) {
      options |= AVAudioSessionCategoryOptionOverrideMutedMicrophoneInterruption;
    }

    if ([option isEqualToString:@"interruptSpokenAudioAndMixWithOthers"]) {
      options |= AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers;
    }
  }

  return options;
}

@end
