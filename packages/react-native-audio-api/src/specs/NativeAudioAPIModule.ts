'use strict';
import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import { AudioDevicesInfo, PermissionStatus } from '../system/types';

interface Spec extends TurboModule {
  install(): boolean;
  getDevicePreferredSampleRate(): number;

  // AVAudioSession management
  setAudioSessionActivity(enabled: boolean): Promise<boolean>;
  setAudioSessionOptions(
    category: string,
    mode: string,
    options: Array<string>,
    allowHaptics: boolean
  ): void;
  disableSessionManagement(): void;

  // Lock Screen Info
  setLockScreenInfo(info: {
    [key: string]: string | boolean | number | undefined;
  }): void;
  resetLockScreenInfo(): void;

  // Remote commands, system events and interruptions
  enableRemoteCommand(name: string, enabled: boolean): void;
  observeAudioInterruptions(enabled: boolean): void;
  activelyReclaimSession(enabled: boolean): void;
  observeVolumeChanges(enabled: boolean): void;

  // Permissions
  requestRecordingPermissions(): Promise<PermissionStatus>;
  checkRecordingPermissions(): Promise<PermissionStatus>;

  // Audio devices
  getDevicesInfo(): Promise<AudioDevicesInfo>;
}

const NativeAudioAPIModule = TurboModuleRegistry.get<Spec>('AudioAPIModule');

export { NativeAudioAPIModule };
