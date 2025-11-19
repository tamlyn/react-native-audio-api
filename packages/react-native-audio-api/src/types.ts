import AudioBuffer from './core/AudioBuffer';

export type ChannelCountMode = 'max' | 'clamped-max' | 'explicit';

export type ChannelInterpretation = 'speakers' | 'discrete';

export type BiquadFilterType =
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

export interface AudioBufferBaseSourceNodeOptions {
  pitchCorrection: boolean;
}

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

export interface TConvolverOptions extends TAudioNodeOptions {
  buffer?: AudioBuffer | null;
  disableNormalization?: boolean;
}

export interface TWebConvolverOptions {
  buffer?: globalThis.AudioBuffer | null;
  normalize?: boolean;
}

export interface TConstantSourceOptions {
  offset?: number;
}

export interface TPeriodicWaveConstraints {
  disableNormalization?: boolean;
}

export interface TPeriodicWaveOptions extends TPeriodicWaveConstraints {
  real: Float32Array;
  imag: Float32Array;
}
