import type {
  AudioEventSubscription,
  RemoteCommandEventName,
  SystemEventCallback,
  SystemEventName,
} from '../events';
import { availabilityWarn } from '../utils';
import IAudioManager from './interface';
import type {
  AudioDevicesInfo,
  LockScreenInfo,
  PermissionStatus,
  SessionOptions,
} from './types';

class NoopSubscription {
  remove(): void {
    // noop
  }
}

class AudioManager implements IAudioManager {
  getDevicePreferredSampleRate(): number {
    availabilityWarn('AudioManager.getDevicePreferredSampleRate', 'web');
    return 0;
  }

  setAudioSessionActivity(_enabled: boolean): Promise<boolean> {
    availabilityWarn('AudioManager.setAudioSessionActivity', 'web');
    return Promise.resolve(false);
  }

  setAudioSessionOptions(_options: SessionOptions) {
    availabilityWarn('AudioManager.setAudioSessionOptions', 'web');
  }

  setLockScreenInfo(_info: LockScreenInfo) {
    availabilityWarn('AudioManager.setLockScreenInfo', 'web');
  }

  resetLockScreenInfo() {
    availabilityWarn('AudioManager.resetLockScreenInfo', 'web');
  }

  observeAudioInterruptions(_enabled: boolean) {
    availabilityWarn('AudioManager.observeAudioInterruptions', 'web');
  }

  activelyReclaimSession(_enabled: boolean): void {
    availabilityWarn('AudioManager.activelyReclaimSession', 'web');
  }

  observeVolumeChanges(_enabled: boolean): void {
    availabilityWarn('AudioManager.observeVolumeChanges', 'web');
  }

  enableRemoteCommand(_name: RemoteCommandEventName, _enabled: boolean): void {
    availabilityWarn('AudioManager.enableRemoteCommand', 'web');
  }

  addSystemEventListener<Name extends SystemEventName>(
    _name: Name,
    _callback: SystemEventCallback<Name>
  ): AudioEventSubscription {
    availabilityWarn('AudioManager.addSystemEventListener', 'web');
    return new NoopSubscription() as unknown as AudioEventSubscription;
  }

  async requestRecordingPermissions(): Promise<PermissionStatus> {
    // TODO: could be implemented some day, some way
    availabilityWarn('AudioManager.requestRecordingPermissions', 'web');
    return Promise.resolve('Denied');
  }

  async checkRecordingPermissions(): Promise<PermissionStatus> {
    // TODO: could be implemented some day, some way
    availabilityWarn('AudioManager.checkRecordingPermissions', 'web');
    return Promise.resolve('Denied');
  }

  async getDevicesInfo(): Promise<AudioDevicesInfo> {
    availabilityWarn('AudioManager.getDevicesInfo', 'web');
    return Promise.resolve({
      availableInputs: [],
      availableOutputs: [],
      currentInputs: [],
      currentOutputs: [],
    });
  }
}

export default new AudioManager();
