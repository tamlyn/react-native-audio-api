import {
  SessionOptions,
  LockScreenInfo,
  RecordingLockScreenInfo,
  PermissionStatus,
  AudioDevicesInfo,
  UiMode,
} from './types';
import {
  SystemEventName,
  SystemEventCallback,
  RemoteCommandEventName,
} from '../events/types';
import { NativeAudioAPIModule } from '../specs';
import { AudioEventEmitter, AudioEventSubscription } from '../events';

if (global.AudioEventEmitter == null) {
  if (!NativeAudioAPIModule) {
    throw new Error(
      `Failed to install react-native-audio-api: The native module could not be found.`
    );
  }

  NativeAudioAPIModule.install();
}

class AudioManager {
  private readonly audioEventEmitter: AudioEventEmitter;
  constructor() {
    this.audioEventEmitter = new AudioEventEmitter(global.AudioEventEmitter);
  }

  getDevicePreferredSampleRate(): number {
    return NativeAudioAPIModule!.getDevicePreferredSampleRate();
  }

  setAudioSessionActivity(enabled: boolean): Promise<boolean> {
    return NativeAudioAPIModule!.setAudioSessionActivity(enabled);
  }

  setAudioSessionOptions(options: SessionOptions) {
    NativeAudioAPIModule!.setAudioSessionOptions(
      options.iosCategory ?? '',
      options.iosMode ?? '',
      options.iosOptions ?? [],
      options.iosAllowHaptics ?? false
    );
  }

  disableSessionManagement() {
    NativeAudioAPIModule!.disableSessionManagement();
  }

  setLockScreenInfo(info: LockScreenInfo) {
    NativeAudioAPIModule!.setLockScreenInfo(info);
  }

  resetLockScreenInfo() {
    NativeAudioAPIModule!.resetLockScreenInfo();
  }

  setRecordingLockScreenInfo(info: RecordingLockScreenInfo) {
    NativeAudioAPIModule!.setRecordingLockScreenInfo(info);
  }

  resetRecordingLockScreenInfo() {
    NativeAudioAPIModule!.resetRecordingLockScreenInfo();
  }

  setUiMode(mode: UiMode) {
    NativeAudioAPIModule!.setUiMode(mode);
  }

  observeAudioInterruptions(enabled: boolean) {
    NativeAudioAPIModule!.observeAudioInterruptions(enabled);
  }

  addRecordingNotificationButtonListener(
    buttonId: string,
    callback: (buttonId: string) => void
  ): string {
    return NativeAudioAPIModule!.addRecordingNotificationButtonListener(
      buttonId,
      callback
    );
  }

  removeRecordingNotificationButtonListener(subscriptionId: string): void {
    NativeAudioAPIModule!.removeRecordingNotificationButtonListener(
      subscriptionId
    );
  }

  /**
   * @param enabled - Whether to actively reclaim the session or not
   * @experimental more aggressively try to reactivate the audio session during interruptions.
   * It is subject to change in the future and might be removed.
   *
   * In some cases (depends on app session settings and other apps using audio) system may never
   * send the `interruption ended` event. This method will check if any other audio is playing
   * and try to reactivate the audio session, as soon as there is "silence".
   * Although this might change the expected behavior.
   *
   * Internally method uses `AVAudioSessionSilenceSecondaryAudioHintNotification` as well as
   * interval polling to check if other audio is playing.
   */
  activelyReclaimSession(enabled: boolean) {
    NativeAudioAPIModule!.activelyReclaimSession(enabled);
  }

  observeVolumeChanges(enabled: boolean) {
    NativeAudioAPIModule!.observeVolumeChanges(enabled);
  }

  enableRemoteCommand(name: RemoteCommandEventName, enabled: boolean) {
    NativeAudioAPIModule!.enableRemoteCommand(name, enabled);
  }

  addSystemEventListener<Name extends SystemEventName>(
    name: Name,
    callback: SystemEventCallback<Name>
  ): AudioEventSubscription {
    return this.audioEventEmitter.addAudioEventListener(name, callback);
  }

  async requestRecordingPermissions(): Promise<PermissionStatus> {
    return NativeAudioAPIModule!.requestRecordingPermissions();
  }

  async checkRecordingPermissions(): Promise<PermissionStatus> {
    return NativeAudioAPIModule!.checkRecordingPermissions();
  }

  async getDevicesInfo(): Promise<AudioDevicesInfo> {
    return NativeAudioAPIModule!.getDevicesInfo();
  }
}

export default new AudioManager();
