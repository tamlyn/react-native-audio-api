import { InvalidStateError, RangeError } from '../../errors';

import type {
  IGenericAudioBuffer,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type { IAudioBufferSourceNode } from '../../types/interfaces';
import { ActionQueue } from '../../utils';
import AudioBuffer from '../AudioBuffer';
import AudioParam from '../AudioParam';

type NativeAudioContext = globalThis.BaseAudioContext;
type NativeAudioBufferSourceNode = globalThis.AudioBufferSourceNode;

interface ScheduleOptions {
  rate?: number;
  active?: boolean;
  output?: number;
  input?: number;
  semitones?: number;
  loopStart?: number;
  loopEnd?: number;
}

interface IStretcherNode extends globalThis.AudioNode {
  channelCount: number;
  channelCountMode: globalThis.ChannelCountMode;
  channelInterpretation: globalThis.ChannelInterpretation;
  context: globalThis.BaseAudioContext;
  numberOfInputs: number;
  numberOfOutputs: number;

  onended:
    | ((this: globalThis.AudioScheduledSourceNode, ev: Event) => unknown)
    | null;
  addEventListener: (
    type: string,
    listener: EventListenerOrEventListenerObject | null,
    options?: boolean | AddEventListenerOptions | undefined
  ) => void;
  dispatchEvent: (event: Event) => boolean;
  removeEventListener: (
    type: string,
    callback: EventListenerOrEventListenerObject | null,
    options?: boolean | EventListenerOptions | undefined
  ) => void;

  addBuffers(channels: Float32Array[]): void;
  dropBuffers(): void;

  schedule(options: ScheduleOptions): void;

  start(
    when?: number,
    offset?: number,
    duration?: number,
    rate?: number,
    semitones?: number
  ): void;

  stop(when?: number): void;

  connect(
    destination: globalThis.AudioNode,
    output?: number,
    input?: number
  ): globalThis.AudioNode;
  connect(destination: globalThis.AudioParam, output?: number): void;

  disconnect(): void;
  disconnect(output: number): void;

  disconnect(destination: globalThis.AudioNode): globalThis.AudioNode;
  disconnect(destination: globalThis.AudioNode, output: number): void;
  disconnect(
    destination: globalThis.AudioNode,
    output: number,
    input: number
  ): void;

  disconnect(destination: globalThis.AudioParam): void;
  disconnect(destination: globalThis.AudioParam, output: number): void;
}

class StretcherNodeAudioParam implements globalThis.AudioParam {
  private _value: number;
  private _setter: (value: number, when?: number) => void;

  public automationRate: AutomationRate;
  public defaultValue: number;
  public maxValue: number;
  public minValue: number;

  constructor(
    value: number,
    setter: (value: number, when?: number) => void,
    automationRate: AutomationRate,
    minValue: number,
    maxValue: number,
    defaultValue: number
  ) {
    this._value = value;
    this.automationRate = automationRate;
    this.minValue = minValue;
    this.maxValue = maxValue;
    this.defaultValue = defaultValue;
    this._setter = setter;
  }

  public get value(): number {
    return this._value;
  }

  public set value(value: number) {
    this._value = value;

    this._setter(value);
  }

  cancelAndHoldAtTime(cancelTime: number): globalThis.AudioParam {
    this._setter(this.defaultValue, cancelTime);
    return this;
  }

  cancelScheduledValues(cancelTime: number): globalThis.AudioParam {
    this._setter(this.defaultValue, cancelTime);
    return this;
  }

  exponentialRampToValueAtTime(
    _value: number,
    _endTime: number
  ): globalThis.AudioParam {
    console.warn(
      'exponentialRampToValueAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  linearRampToValueAtTime(
    _value: number,
    _endTime: number
  ): globalThis.AudioParam {
    console.warn(
      'linearRampToValueAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  setTargetAtTime(
    _target: number,
    _startTime: number,
    _timeConstant: number
  ): globalThis.AudioParam {
    console.warn(
      'setTargetAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  setValueAtTime(value: number, startTime: number): globalThis.AudioParam {
    this._setter(value, startTime);
    return this;
  }

  setValueCurveAtTime(
    _values: Float32Array,
    _startTime: number,
    _duration: number
  ): globalThis.AudioParam {
    console.warn(
      'setValueCurveAtTime is not implemented for pitch correction mode'
    );
    return this;
  }
}

type WebBufferSourceNode = NativeAudioBufferSourceNode | IStretcherNode;

// Note: this class do not extend from AudioScheduledSourceNode or AudioNode
// because of the promise based access nature of the stretcher node
export default class AudioBufferSourceNode<
  TContext extends IGenericBaseAudioContext,
  NNode extends WebBufferSourceNode = NativeAudioBufferSourceNode,
> implements IAudioBufferSourceNode<TContext>
{
  readonly playbackRate: AudioParam<TContext, NativeAudioContext>;
  readonly detune: AudioParam<TContext, NativeAudioContext>;

  private mPitchCorrection: boolean;

  private mLoop: boolean = false;
  private mLoopStart: number = 0;
  private mLoopEnd: number = 0;

  private mBuffer: AudioBuffer | null = null;

  private actionQueue = new ActionQueue();

  private hasBeenStarted = false;

  constructor(
    context: TContext,
    nodePromise: Promise<NNode>,
    pitchCorrection: boolean
  ) {
    this.mPitchCorrection = pitchCorrection;

    if (!pitchCorrection) {
      this.playbackRate = new AudioParam(null, context);
      this.detune = new AudioParam(null, context);
    }
  }
}
