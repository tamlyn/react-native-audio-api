export type ContextState = 'running' | 'closed' | `suspended`;

export type OscillatorType =
  | 'sine'
  | 'square'
  | 'sawtooth'
  | 'triangle'
  | 'custom';

export interface PeriodicWaveConstraints {
  disableNormalization: boolean;
}

export interface AudioContextOptions {
  sampleRate?: number;
  initSuspended?: boolean;
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

export interface AudioBufferBaseSourceNodeOptions {
  pitchCorrection: boolean;
}

export type ProcessorMode = 'processInPlace' | 'processThrough';
