export type IOSCategory =
  | 'record'
  | 'ambient'
  | 'playback'
  | 'multiRoute'
  | 'soloAmbient'
  | 'playAndRecord';

export type IOSMode =
  | 'default'
  | 'gameChat'
  | 'videoChat'
  | 'voiceChat'
  | 'measurement'
  | 'voicePrompt'
  | 'spokenAudio'
  | 'moviePlayback'
  | 'videoRecording';

export type IOSOption =
  | 'duckOthers'
  | 'allowAirPlay'
  | 'mixWithOthers'
  | 'allowBluetooth'
  | 'defaultToSpeaker'
  | 'allowBluetoothA2DP'
  | 'overrideMutedMicrophoneInterruption'
  | 'interruptSpokenAudioAndMixWithOthers';

export interface SessionOptions {
  iosMode?: IOSMode;
  iosOptions?: IOSOption[];
  iosCategory?: IOSCategory;
  iosAllowHaptics?: boolean;
}

export type MediaState = 'state_playing' | 'state_paused';

// eslint-disable-next-line @typescript-eslint/no-duplicate-enum-values
export enum UiMode {
  PLAYBACK = 'PLAYBACK',
  RECORDING = 'RECORDING',
}

interface BaseLockScreenInfo {
  [key: string]: string | boolean | number | undefined;
}

export interface LockScreenInfo extends BaseLockScreenInfo {
  title?: string;
  artwork?: string;
  artist?: string;
  album?: string;
  duration?: number;
  description?: string; // android only
  state?: MediaState;
  speed?: number;
  elapsedTime?: number;
}

export interface RecordingLockScreenInfo {
  title?: string;
  description?: string;
}

export enum ForegroundAction {
  START_FOREGROUND = 'START_FOREGROUND',
  STOP_FOREGROUND = 'STOP_FOREGROUND',
}

export type PermissionStatus = 'Undetermined' | 'Denied' | 'Granted';

export interface AudioDeviceInfo {
  name: string;
  category: string;
}

export type AudioDeviceList = AudioDeviceInfo[];

export interface AudioDevicesInfo {
  availableInputs: AudioDeviceList;
  availableOutputs: AudioDeviceList;
  currentInputs: AudioDeviceList; // iOS only
  currentOutputs: AudioDeviceList; // iOS only
}
