import { InvalidAccessError, NotSupportedError } from '../errors';
import { IBaseAudioContext } from '../interfaces';
import { ContextState, AudioWorkletRuntime } from '../types';
import { assertWorkletsEnabled } from '../utils';
import WorkletSourceNode from './WorkletSourceNode';
import WorkletProcessingNode from './WorkletProcessingNode';
import AnalyserNode from './AnalyserNode';
import AudioBuffer from './AudioBuffer';
import AudioBufferQueueSourceNode from './AudioBufferQueueSourceNode';
import ConvolverNode from './ConvolverNode';
import AudioBufferSourceNode from './AudioBufferSourceNode';
import AudioDestinationNode from './AudioDestinationNode';
import BiquadFilterNode from './BiquadFilterNode';
import ConstantSourceNode from './ConstantSourceNode';
import GainNode from './GainNode';
import OscillatorNode from './OscillatorNode';
import PeriodicWave from './PeriodicWave';
import RecorderAdapterNode from './RecorderAdapterNode';
import StereoPannerNode from './StereoPannerNode';
import StreamerNode from './StreamerNode';
import WorkletNode from './WorkletNode';
import { decodeAudioData, decodePCMInBase64 } from './AudioDecoder';

export default class BaseAudioContext {
  readonly destination: AudioDestinationNode;
  readonly sampleRate: number;
  readonly context: IBaseAudioContext;

  constructor(context: IBaseAudioContext) {
    this.context = context;
    this.destination = new AudioDestinationNode(this, context.destination);
    this.sampleRate = context.sampleRate;
  }

  public get currentTime(): number {
    return this.context.currentTime;
  }

  public get state(): ContextState {
    return this.context.state;
  }

  public async decodeAudioData(
    input: string | ArrayBuffer,
    sampleRate?: number
  ): Promise<AudioBuffer> {
    if (!(typeof input === 'string' || input instanceof ArrayBuffer)) {
      throw new TypeError('Input must be a string or ArrayBuffer');
    }
    return await decodeAudioData(input, sampleRate ?? this.sampleRate);
  }

  public async decodePCMInBase64(
    base64String: string,
    inputSampleRate: number,
    inputChannelCount: number,
    isInterleaved: boolean = true
  ): Promise<AudioBuffer> {
    return await decodePCMInBase64(
      base64String,
      inputSampleRate,
      inputChannelCount,
      isInterleaved
    );
  }

  createWorkletNode(
    callback: (audioData: Array<Float32Array>, channelCount: number) => void,
    bufferLength: number,
    inputChannelCount: number,
    workletRuntime: AudioWorkletRuntime = 'AudioRuntime'
  ): WorkletNode {
    if (inputChannelCount < 1 || inputChannelCount > 32) {
      throw new NotSupportedError(
        `The number of input channels provided (${inputChannelCount}) can not be less than 1 or greater than 32`
      );
    }
    if (bufferLength < 1) {
      throw new NotSupportedError(
        `The buffer length provided (${bufferLength}) can not be less than 1`
      );
    }
    assertWorkletsEnabled();
    return new WorkletNode(
      this,
      workletRuntime,
      callback,
      bufferLength,
      inputChannelCount
    );
  }

  createWorkletProcessingNode(
    callback: (
      inputData: Array<Float32Array>,
      outputData: Array<Float32Array>,
      framesToProcess: number,
      currentTime: number
    ) => void,
    workletRuntime: AudioWorkletRuntime = 'AudioRuntime'
  ): WorkletProcessingNode {
    assertWorkletsEnabled();
    return new WorkletProcessingNode(this, workletRuntime, callback);
  }

  createWorkletSourceNode(
    callback: (
      audioData: Array<Float32Array>,
      framesToProcess: number,
      currentTime: number,
      startOffset: number
    ) => void,
    workletRuntime: AudioWorkletRuntime = 'AudioRuntime'
  ): WorkletSourceNode {
    assertWorkletsEnabled();
    return new WorkletSourceNode(this, workletRuntime, callback);
  }

  createRecorderAdapter(): RecorderAdapterNode {
    return new RecorderAdapterNode(this);
  }

  createOscillator(): OscillatorNode {
    return new OscillatorNode(this);
  }

  createStreamer(): StreamerNode {
    return new StreamerNode(this);
  }

  createConstantSource(): ConstantSourceNode {
    return new ConstantSourceNode(this);
  }

  createGain(): GainNode {
    return new GainNode(this);
  }

  createStereoPanner(): StereoPannerNode {
    return new StereoPannerNode(this);
  }

  createBiquadFilter(): BiquadFilterNode {
    return new BiquadFilterNode(this);
  }

  createBufferSource(): AudioBufferSourceNode {
    return new AudioBufferSourceNode(this);
  }

  createBufferQueueSource(): AudioBufferQueueSourceNode {
    return new AudioBufferQueueSourceNode(this);
  }

  createBuffer(
    numberOfChannels: number,
    length: number,
    sampleRate: number
  ): AudioBuffer {
    if (numberOfChannels < 1 || numberOfChannels >= 32) {
      throw new NotSupportedError(
        `The number of channels provided (${numberOfChannels}) is outside the range [1, 32]`
      );
    }

    if (length <= 0) {
      throw new NotSupportedError(
        `The number of frames provided (${length}) is less than or equal to the minimum bound (0)`
      );
    }

    if (sampleRate < 8000 || sampleRate > 96000) {
      throw new NotSupportedError(
        `The sample rate provided (${sampleRate}) is outside the range [8000, 96000]`
      );
    }

    return new AudioBuffer(this, { numberOfChannels, length, sampleRate });
  }

  createPeriodicWave(
    real: Float32Array,
    imag: Float32Array,
    constraints?: PeriodicWaveConstraints
  ): PeriodicWave {
    if (real.length !== imag.length) {
      throw new InvalidAccessError(
        `The lengths of the real (${real.length}) and imaginary (${imag.length}) arrays must match.`
      );
    }
    return new PeriodicWave(this, { real, imag, ...constraints });
  }

  createAnalyser(): AnalyserNode {
    return new AnalyserNode(this);
  }

  createConvolver(): ConvolverNode {
    return new ConvolverNode(this);
  }
}
