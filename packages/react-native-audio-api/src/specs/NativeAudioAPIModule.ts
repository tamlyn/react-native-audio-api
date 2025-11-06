'use strict';
import { TurboModuleRegistry } from 'react-native';
import type { TurboModule } from 'react-native';
import {
  PermissionStatus,
  AudioDevicesInfo,
  RecordingLockScreenInfo,
} from '../system/types';

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

  // Recording Lock Screen Info
  setRecordingLockScreenInfo(info: {
    title?: string;
    description?: string;
  }): void;
  resetRecordingLockScreenInfo(): void;
  setUiMode(mode: string): void; // 'PLAYBACK' | 'RECORDING'

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
