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

export type ContextState = 'running' | 'closed' | 'suspended';

export type AudioWorkletRuntime = 'AudioRuntime' | 'UIRuntime';

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

export enum FileDirectory {
  Document = 1,
  Cache = 2,
}

export enum IOSFormat {
  Wav = 1,
  Caf = 2,
  M4A = 3,
  Flac = 4,
}

export enum IOSAudioQuality {
  Min = 1,
  Low = 2,
  Medium = 3,
  High = 4,
  Max = 5,
}

export enum AndroidFormat {
  Wav = 1,
  Caf = 2,
  M4A = 3,
  Flac = 4,
}

export enum FlacCompressionLevel {
  L0 = 1,
  L1 = 2,
  L2 = 3,
  L3 = 4,
  L4 = 5,
  L5 = 6,
  L6 = 7,
  L7 = 8,
  L8 = 9,
}

export enum BitDepth {
  Bit16 = 1,
  Bit24 = 2,
  Bit32 = 3,
}

export interface AudioRecorderFileOptionsIOS {
  format?: IOSFormat;
  quality?: IOSAudioQuality;
  flacCompressionLevel?: FlacCompressionLevel;
}

export interface AudioRecorderFileOptionsAndroid {
  format?: AndroidFormat;
}

export interface AudioRecorderFileOptions {
  directory?: FileDirectory;
  sampleRate?: number;
  channels?: number;
  bitRate?: number;
  bitDepth?: BitDepth;
  ios?: AudioRecorderFileOptionsIOS;
  android?: AudioRecorderFileOptionsAndroid;
}

export type WindowType = 'blackman' | 'hann';

export interface AudioBufferBaseSourceNodeOptions {
  pitchCorrection: boolean;
}

export type ProcessorMode = 'processInPlace' | 'processThrough';

export interface AudioRecorderCallbackOptions {
  /**
   * The desired sample rate (in Hz) for audio buffers delivered to the
   * recording callback. Common values include 44100 or 48000 Hz. The actual
   * sample rate may differ depending on hardware and system capabilities.
   */
  sampleRate: number;

  /**
   * The preferred size of each audio buffer, expressed as the number of samples
   * per channel. Smaller buffers reduce latency but increase CPU load, while
   * larger buffers improve efficiency at the cost of higher latency.
   */
  bufferLength: number;

  /**
   * The desired number of audio channels per buffer. Typically 1 for mono or 2
   * for stereo recordings.
   */
  channelCount: number;
}

export interface FileInfo {
  path: string;
  size: number;
  duration: number;
}
