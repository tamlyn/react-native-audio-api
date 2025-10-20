import { AudioEventCallback, AudioEventName } from './events/types';
import { ContextState } from './types';

export type WorkletNodeCallback = (
  audioData: Array<ArrayBuffer>,
  channelCount: number
) => void;

export type WorkletSourceNodeCallback = (
  audioData: Array<ArrayBuffer>,
  framesToProcess: number,
  currentTime: number,
  startOffset: number
) => void;

export type WorkletProcessingNodeCallback = (
  inputData: Array<ArrayBuffer>,
  outputData: Array<ArrayBuffer>,
  framesToProcess: number,
  currentTime: number
) => void;

export type ShareableWorkletCallback =
  | WorkletNodeCallback
  | WorkletSourceNodeCallback
  | WorkletProcessingNodeCallback;

export interface IBaseAudioContext {
  readonly destination: IAudioDestinationNode;
  readonly state: ContextState;
  readonly sampleRate: number;
  readonly currentTime: number;
  readonly decoder: IAudioDecoder;
  readonly stretcher: IAudioStretcher;

  createRecorderAdapter(): IRecorderAdapterNode;
  createWorkletSourceNode(
    shareableWorklet: ShareableWorkletCallback,
    shouldUseUiRuntime: boolean
  ): IWorkletSourceNode;
  createWorkletNode(
    shareableWorklet: ShareableWorkletCallback,
    shouldUseUiRuntime: boolean,
    bufferLength: number,
    inputChannelCount: number
  ): IWorkletNode;
  createWorkletProcessingNode(
    shareableWorklet: ShareableWorkletCallback,
    shouldUseUiRuntime: boolean
  ): IWorkletProcessingNode;
  createOscillator(): IOscillatorNode;
  createConstantSource(): IConstantSourceNode;
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

export interface IWorkletNode extends IAudioNode {}

export interface IWorkletSourceNode extends IAudioScheduledSourceNode {}

export interface IWorkletProcessingNode extends IAudioNode {}

export interface IAudioRecorder {
  start: () => void;
  stop: () => void;
  connect: (node: IRecorderAdapterNode) => void;
  disconnect: () => void;

  // passing subscriptionId(uint_64 in cpp, string in js) to the cpp
  onAudioReady: string;
}

export interface IAudioDecoder {
  decodeWithMemoryBlock: (
    arrayBuffer: ArrayBuffer,
    sampleRate?: number
  ) => Promise<IAudioBuffer>;
  decodeWithFilePath: (
    sourcePath: string,
    sampleRate?: number
  ) => Promise<IAudioBuffer>;
  decodeWithPCMInBase64: (
    b64: string,
    inputSampleRate: number,
    inputChannelCount: number,
    interleaved?: boolean
  ) => Promise<IAudioBuffer>;
}

export interface IAudioStretcher {
  changePlaybackSpeed: (
    arrayBuffer: AudioBuffer,
    playbackSpeed: number
  ) => Promise<IAudioBuffer>;
}

export interface IAudioEventEmitter {
  addAudioEventListener<Name extends AudioEventName>(
    name: Name,
    callback: AudioEventCallback<Name>
  ): string;
  removeAudioEventListener<Name extends AudioEventName>(
    name: Name,
    subscriptionId: string
  ): void;
}
