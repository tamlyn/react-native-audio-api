import type {
  AudioEventSubscription,
  RemoteCommandEventName,
  SystemEventCallback,
  SystemEventName,
} from '../events';

import type {
  AudioDevicesInfo,
  LockScreenInfo,
  PermissionStatus,
  SessionOptions,
} from './types';

export default interface IAudioManager {
  getDevicePreferredSampleRate(): number;
  setAudioSessionActivity(enabled: boolean): Promise<boolean>;
  setAudioSessionOptions(options: SessionOptions): void; // TODO: should return Promise<boolean> ????
  setLockScreenInfo(info: LockScreenInfo): void;
  resetLockScreenInfo(): void;
  observeAudioInterruptions(enabled: boolean): void;
  activelyReclaimSession(enabled: boolean): void;
  observeVolumeChanges(enabled: boolean): void;
  enableRemoteCommand(name: RemoteCommandEventName, enabled: boolean): void;

  addSystemEventListener<Name extends SystemEventName>(
    name: Name,
    callback: SystemEventCallback<Name>
  ): AudioEventSubscription;

  requestRecordingPermissions(): Promise<PermissionStatus>;
  checkRecordingPermissions(): Promise<PermissionStatus>;
  getDevicesInfo(): Promise<AudioDevicesInfo>;
}
