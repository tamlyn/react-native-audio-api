import { InvalidStateError, RangeError } from '../../errors';

import type { ChannelCountMode, ChannelInterpretation } from '../../types';
import type { IGenericBaseAudioContext } from '../../types/generics';
import type {
  IAudioBufferSourceNode,
  OnEndedEventCallback,
  OnPositionChangedEventCallback,
} from '../../types/interfaces';
import { availabilityWarn } from '../../utils';
import AudioBuffer from '../AudioBuffer';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

import { IStretcherNode, StretcherNodeAudioParam } from './ABSNWeb';

type NativeAudioContext = globalThis.BaseAudioContext;
type NativeAudioBufferSourceNode = globalThis.AudioBufferSourceNode;
type NativeAudioParam = globalThis.AudioParam;
type NativeAudioNode = globalThis.AudioNode;

type WebAudioNode = NativeAudioNode | IStretcherNode;
type WebBufferSourceNode = NativeAudioBufferSourceNode | IStretcherNode;
type WebAudioParam = NativeAudioParam | StretcherNodeAudioParam;

function isStretcherNode(
  node: WebBufferSourceNode,
  pitchCorrection: boolean
): node is IStretcherNode {
  return pitchCorrection && (node as IStretcherNode).schedule !== undefined;
}

function isNativeNode(
  node: WebBufferSourceNode,
  pitchCorrection: boolean
): node is NativeAudioBufferSourceNode {
  return (
    !pitchCorrection &&
    (node as NativeAudioBufferSourceNode).buffer !== undefined
  );
}

export default class AudioBufferSourceNode<
  TContext extends IGenericBaseAudioContext,
> implements IAudioBufferSourceNode<TContext>
{
  readonly playbackRate: AudioParam<TContext, NativeAudioContext>;
  readonly detune: AudioParam<TContext, NativeAudioContext>;
  readonly numberOfInputs: number = 0;
  readonly numberOfOutputs: number = 1;
  channelCount: number = 0;
  readonly channelCountMode: ChannelCountMode = 'explicit';
  readonly channelInterpretation: ChannelInterpretation = 'speakers';
  readonly context: TContext;

  private mPitchCorrection: boolean;

  private mLoop: boolean = false;
  private mLoopStart: number = 0;
  private mLoopEnd: number = 0;
  private mNode: WebBufferSourceNode;
  private mLoopSkip: boolean = false;

  private mBuffer: AudioBuffer | null = null;
  private mHasBeenStarted = false;
  private mCallback: OnEndedEventCallback | null = null;

  constructor(
    context: TContext,
    node: WebBufferSourceNode,
    pitchCorrection: boolean
  ) {
    this.mNode = node;
    this.context = context;
    this.mPitchCorrection = pitchCorrection;

    if (isNativeNode(node, pitchCorrection)) {
      this.detune = new AudioParam<TContext, NativeAudioContext>(
        node.detune,
        context
      );

      this.playbackRate = new AudioParam<TContext, NativeAudioContext>(
        node.playbackRate,
        context
      );
      return;
    }

    this.detune = new AudioParam<TContext, NativeAudioContext>(
      new StretcherNodeAudioParam(
        0,
        this.setDetune.bind(this),
        'a-rate',
        -1200,
        1200,
        0
      ),
      context
    );

    this.playbackRate = new AudioParam<TContext, NativeAudioContext>(
      new StretcherNodeAudioParam(
        1,
        this.setPlaybackRate.bind(this),
        'a-rate',
        0,
        Infinity,
        1
      ),
      context
    );
  }

  setDetune(value: number, _when: number = 0) {
    // This method should be called only for stretcher node (internally used with StretcherNodeAudioParam)
    // for not-started nodes, we don't have to do anything. Start method will pick up the initial detune value.
    if (
      !isStretcherNode(this.mNode, this.mPitchCorrection) ||
      !this.mHasBeenStarted
    ) {
      return;
    }

    this.mNode.schedule({
      semitones: Math.floor(Math.max(-12, Math.min(12, value / 100))),
      output: 0,
    });
  }

  setPlaybackRate(value: number, when: number = 0) {
    if (
      !isStretcherNode(this.mNode, this.mPitchCorrection) ||
      !this.mHasBeenStarted
    ) {
      return;
    }

    this.mNode.schedule({
      rate: value,
      output: when,
    });
  }

  get buffer(): AudioBuffer | null {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      return this.mBuffer;
    }

    const buffer = this.mNode.buffer;

    if (!buffer) {
      return null;
    }

    return new AudioBuffer(buffer);
  }

  set buffer(value: AudioBuffer | null) {
    this.channelCount = value ? value.numberOfChannels : 0;
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      this.mBuffer = value;

      const stretcher = this.mNode;
      stretcher.dropBuffers();

      if (!value) {
        return;
      }

      const channelArrays: Float32Array[] = [];

      for (let i = 0; i < value.numberOfChannels; i++) {
        channelArrays.push(value.getChannelData(i));
      }

      stretcher.addBuffers(channelArrays);
      return;
    }

    if (this.mHasBeenStarted) {
      throw new InvalidStateError(
        'Cannot set buffer after the source has been started.'
      );
    }

    this.mNode.buffer = value ? value.buffer : null;
  }

  get loop(): boolean {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      return this.mLoop;
    }

    return this.mNode.loop;
  }

  set loop(value: boolean) {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      this.mLoop = value;
      return;
    }

    this.mNode.loop = value;
  }

  get loopStart(): number {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      return this.mLoopStart;
    }

    return this.mNode.loopStart;
  }

  set loopStart(value: number) {
    if (value < 0) {
      throw new RangeError('loopStart must be non-negative');
    }

    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      this.mLoopStart = value;
      return;
    }

    this.mNode.loopStart = value;
  }

  get loopEnd(): number {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      return this.mLoopEnd;
    }

    return this.mNode.loopEnd;
  }

  set loopEnd(value: number) {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      this.mLoopEnd = value;
      return;
    }

    this.mNode.loopEnd = value;
  }

  get loopSkip(): boolean {
    availabilityWarn('AudioBufferSourceNode.loopSkip', 'web');
    return this.mLoopSkip;
  }

  set loopSkip(value: boolean) {
    availabilityWarn('AudioBufferSourceNode.loopSkip', 'web');
    this.mLoopSkip = value;
  }

  public start(when?: number, offset?: number, duration?: number): void {
    if (when && when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    if (offset && offset < 0) {
      throw new RangeError(
        `offset must be a finite non-negative number: ${offset}`
      );
    }

    if (duration && duration < 0) {
      throw new RangeError(
        `duration must be a finite non-negative number: ${duration}`
      );
    }

    if (this.mHasBeenStarted) {
      throw new InvalidStateError('Cannot call start more than once');
    }

    this.mHasBeenStarted = true;

    if (!isStretcherNode(this.mNode, this.mPitchCorrection)) {
      this.mNode.start(when, offset, duration);
      return;
    }

    const startAt =
      !when || when < this.mNode.context.currentTime
        ? this.mNode.context.currentTime
        : when;

    if (this.loop && this.mLoopStart !== -1 && this.mLoopEnd !== -1) {
      // Schedule looping if needed
      this.mNode.schedule({
        loopStart: this.mLoopStart,
        loopEnd: this.mLoopEnd,
      });
    }

    this.mNode.start(
      startAt,
      offset,
      duration,
      this.playbackRate.value,
      Math.floor(Math.max(-12, Math.min(12, this.detune.value / 100)))
    );
  }

  public stop(when: number = 0): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    if (!this.mHasBeenStarted) {
      throw new InvalidStateError(
        'Cannot call stop without calling start first'
      );
    }

    this.mNode.stop(when);
  }

  public get onPositionChanged(): OnPositionChangedEventCallback | undefined {
    availabilityWarn('AudioBufferSourceNode.onPositionChanged', 'web');
    return undefined;
  }

  public set onPositionChanged(
    _callback: OnPositionChangedEventCallback | null
  ) {
    availabilityWarn('AudioBufferSourceNode.onPositionChanged', 'web');
  }

  public set onPositionChangedInterval(value: number) {
    availabilityWarn('AudioBufferSourceNode.onPositionChangedInterval', 'web');
  }

  public get onPositionChangedInterval(): number {
    availabilityWarn('AudioBufferSourceNode.onPositionChangedInterval', 'web');
    return 0;
  }

  public get onEnded(): OnEndedEventCallback | undefined {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      availabilityWarn('AudioBufferSourceNode.onEnded', 'web');
    }

    return this.mCallback || undefined;
  }

  public set onEnded(callback: OnEndedEventCallback | null) {
    if (isStretcherNode(this.mNode, this.mPitchCorrection)) {
      availabilityWarn('AudioBufferSourceNode.onEnded', 'web');
      return;
    }

    this.mCallback = callback;

    this.mNode.onended = () => {
      this.mCallback?.({ bufferId: undefined });
    };
  }

  public connect<ONode extends AudioNode<TContext, NativeAudioContext>>(
    destination: ONode
  ): ONode;

  public connect(destination: AudioParam<TContext, NativeAudioContext>): void;

  public connect<ONode extends AudioNode<TContext, NativeAudioContext>>(
    destination: ONode | AudioParam<TContext, NativeAudioContext>
  ): ONode | void {
    if (this.context !== destination.context) {
      throw new Error(
        'Source and destination are from different BaseAudioContexts'
      );
    }

    if (destination instanceof AudioParam) {
      this.mNode.connect(destination.param as WebAudioParam);
      return;
    }

    // TS hack since due to how Stretcher vs NativeAudioNode types are defined
    // we cannot extend/subclass the AudioNode here (where node is protected)
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    this.mNode.connect((destination as any).node as WebAudioNode);
    return destination;
  }

  public disconnect(): void;

  public disconnect<ONode extends AudioNode<TContext, NativeAudioContext>>(
    destination: ONode
  ): void;

  public disconnect(
    destination: AudioParam<TContext, NativeAudioContext>
  ): void;

  public disconnect(
    destination?:
      | AudioNode<TContext, NativeAudioContext>
      | AudioParam<TContext, NativeAudioContext>
  ): void {
    if (destination instanceof AudioParam) {
      this.mNode.disconnect(destination.param as WebAudioParam);
      return;
    }

    // TS hack since due to how Stretcher vs NativeAudioNode types are defined
    // we cannot extend/subclass the AudioNode here (where node is protected)
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    this.mNode.disconnect((destination as any)?.node as WebAudioNode);
  }
}
