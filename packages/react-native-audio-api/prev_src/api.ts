import type {
  IAudioContext,
  IAudioRecorder,
  IOfflineAudioContext,
} from './interfaces';
import { NativeAudioAPIModule } from './specs';
import { AudioRecorderOptions } from './types';

/* eslint-disable no-var */
declare global {
  var createAudioContext: (
    sampleRate: number,
    initSuspended: boolean
  ) => IAudioContext;
  var createOfflineAudioContext: (
    numberOfChannels: number,
    length: number,
    sampleRate: number
  ) => IOfflineAudioContext;

  var createAudioRecorder: (options: AudioRecorderOptions) => IAudioRecorder;
}
/* eslint-disable no-var */

if (
  global.createAudioContext == null ||
  global.createOfflineAudioContext == null ||
  global.createAudioRecorder == null ||
  global.AudioEventEmitter == null
) {
  if (!NativeAudioAPIModule) {
    throw new Error(
      `Failed to install react-native-audio-api: The native module could not be found.`
    );
  }

  NativeAudioAPIModule.install();
}

// export { default as AnalyserNode } from './core/AnalyserNode';
// export { default as AudioBuffer } from './core/AudioBuffer';
// export { default as AudioBufferQueueSourceNode } from './core/AudioBufferQueueSourceNode';
// export { default as AudioBufferSourceNode } from './core/AudioBufferSourceNode';
// export { default as AudioContext } from './core/AudioContext';
// export { default as AudioDestinationNode } from './core/AudioDestinationNode';
// export { default as AudioNode } from './core/AudioNode';
// export { default as AudioParam } from './core/AudioParam';
// export { default as AudioRecorder } from './core/AudioRecorder';
// export { default as AudioScheduledSourceNode } from './core/AudioScheduledSourceNode';
// export { default as BaseAudioContext } from './core/BaseAudioContext';
// export { default as BiquadFilterNode } from './core/BiquadFilterNode';
// export { default as GainNode } from './core/GainNode';
// export { default as OfflineAudioContext } from './core/OfflineAudioContext';
// export { default as OscillatorNode } from './core/OscillatorNode';
// export { default as RecorderAdapterNode } from './core/RecorderAdapterNode';
// export { default as StereoPannerNode } from './core/StereoPannerNode';
// export { default as StreamerNode } from './core/StreamerNode';
// export { default as WorkletNode } from './core/WorkletNode';
// export { default as useSystemVolume } from './hooks/useSytemVolume';
// export { default as AudioManager } from './system';

// export {
//   BiquadFilterType,
//   ChannelCountMode,
//   ChannelInterpretation,
//   ContextState,
//   OscillatorType,
//   PeriodicWaveConstraints,
//   WindowType,
// } from './types';

// export {
//   IndexSizeError,
//   InvalidAccessError,
//   InvalidStateError,
//   NotSupportedError,
//   RangeError,
// } from './errors';
