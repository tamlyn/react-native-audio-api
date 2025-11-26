import AudioBuffer from './core/AudioBuffer';
import PeriodicWave from './core/PeriodicWave';

export type ChannelCountMode = 'max' | 'clamped-max' | 'explicit';

export type ChannelInterpretation = 'speakers' | 'discrete';

type BiquadFilterType =
  | 'lowpass'
  | 'highpass'
  | 'bandpass'
  | 'lowshelf'
  | 'highshelf'
  | 'peaking'
  | 'notch'
  | 'allpass';

export type ContextState = 'running' | 'closed' | `suspended`;

export type AudioWorkletRuntime = 'AudioRuntime' | 'UIRuntime';

export type OscillatorType =
  | 'sine'
  | 'square'
  | 'sawtooth'
  | 'triangle'
  | 'custom';

export interface AudioContextOptions {
  sampleRate?: number;
}

export interface OfflineAudioContextOptions {
  numberOfChannels: number;
  length: number;
  sampleRate: number;
}

export interface AudioRecorderOptions {
  sampleRate: number;
  bufferLengthInSamples: number;
}

export type WindowType = 'blackman' | 'hann';

export type ProcessorMode = 'processInPlace' | 'processThrough';

export interface TAudioNodeOptions {
  channelCount?: number;
  channelCountMode?: ChannelCountMode;
  channelInterpretation?: ChannelInterpretation;
}

export interface TGainOptions extends TAudioNodeOptions {
  gain?: number;
}

export interface TSteroPannerOptions extends TAudioNodeOptions {
  pan?: number;
}

export interface TAnalyserOptions extends TAudioNodeOptions {
  fftSize?: number;
  minDecibels?: number;
  maxDecibels?: number;
  smoothingTimeConstant?: number;
}

export interface TBiquadFilterOptions extends TAudioNodeOptions {
  type?: BiquadFilterType;
  frequency?: number;
  detune?: number;
  Q?: number;
  gain?: number;
}

export interface TOscillatorOptions {
  type?: OscillatorType;
  frequency?: number;
  detune?: number;
  periodicWave?: PeriodicWave;
}

export interface TBaseAudioBufferSourceOptions {
  detune?: number;
  playbackRate?: number;
  pitchCorrection?: boolean;
}

export interface TAudioBufferSourceOptions
  extends TBaseAudioBufferSourceOptions {
  buffer?: AudioBuffer;
  loop?: boolean;
  loopStart?: number;
  loopEnd?: number;
}

export interface TConvolverOptions extends TAudioNodeOptions {
  buffer?: AudioBuffer;
  disableNormalization?: boolean;
}

export interface TWebConvolverOptions {
  buffer?: globalThis.AudioBuffer | null;
  normalize?: boolean;
}

export interface TConstantSourceOptions {
  offset?: number;
}

export interface TStreamerOptions {
  streamPath?: string;
}

export interface TPeriodicWaveConstraints {
  disableNormalization?: boolean;
}

export interface TPeriodicWaveOptions extends TPeriodicWaveConstraints {
  real: Float32Array;
  imag: Float32Array;
}
