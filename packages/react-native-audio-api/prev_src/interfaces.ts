import { ContextState } from './types';

export type ShareableWorkletCallback = (
  audioBuffers: Array<ArrayBuffer>,
  channelCount: number
) => void;

export interface IBaseAudioContext {
  readonly destination: IAudioDestinationNode;
  readonly state: ContextState;
  readonly sampleRate: number;
  readonly currentTime: number;

  createRecorderAdapter(): IRecorderAdapterNode;
  createWorkletNode(
    shareableWorklet: ShareableWorkletCallback,
    bufferLength: number,
    inputChannelCount: number
  ): IWorkletNode;
  createOscillator(): IOscillatorNode;
  createGain(): IGainNode;
  createStereoPanner(): IStereoPannerNode;
  createBiquadFilter: () => IBiquadFilterNode;
  createBufferSource: (pitchCorrection: boolean) => IAudioBufferSourceNode;
  createBufferQueueSource: (
    pitchCorrection: boolean
  ) => IAudioBufferQueueSourceNode;
  createBuffer: (
    channels: number,
    length: number,
    sampleRate: number
  ) => IAudioBuffer;
  createPeriodicWave: (
    real: Float32Array,
    imag: Float32Array,
    disableNormalization: boolean
  ) => IPeriodicWave;
  createAnalyser: () => IAnalyserNode;
  decodeAudioDataSource: (sourcePath: string) => Promise<IAudioBuffer>;
  decodeAudioData: (arrayBuffer: ArrayBuffer) => Promise<IAudioBuffer>;
  decodePCMAudioDataInBase64: (
    b64: string,
    playbackRate: number
  ) => Promise<IAudioBuffer>;
  createStreamer: () => IStreamerNode;
}

export interface IAudioContext extends IBaseAudioContext {
  close(): Promise<void>;
  resume(): Promise<boolean>;
  suspend(): Promise<boolean>;
}

export interface IOfflineAudioContext extends IBaseAudioContext {
  resume(): Promise<void>;
  suspend(suspendTime: number): Promise<void>;
  startRendering(): Promise<IAudioBuffer>;
}

export interface IStreamerNode extends IAudioNode {
  initialize(streamPath: string): boolean;
}

export interface IRecorderAdapterNode extends IAudioNode {}

export interface IWorkletNode extends IAudioNode {}

export interface IAudioRecorder {
  start: () => void;
  stop: () => void;
  connect: (node: IRecorderAdapterNode) => void;
  disconnect: () => void;

  // passing subscriptionId(uint_64 in cpp, string in js) to the cpp
  onAudioReady: string;
}
