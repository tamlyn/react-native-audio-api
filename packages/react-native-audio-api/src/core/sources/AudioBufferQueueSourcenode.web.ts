import type { ChannelCountMode, ChannelInterpretation } from '../../types';
import type { IGenericBaseAudioContext } from '../../types/generics';
import type {
  IAudioBufferQueueSourceNode,
  OnEndedEventCallback,
  OnPositionChangedEventCallback,
} from '../../types/interfaces';
import { availabilityWarn } from '../../utils';
import AudioBuffer from '../AudioBuffer';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

type NativeContext = globalThis.BaseAudioContext;

export default class AudioBufferQueueSourceNode<
  TContext extends IGenericBaseAudioContext,
> implements IAudioBufferQueueSourceNode<TContext, AudioBuffer>
{
  readonly context: TContext;
  readonly numberOfInputs = 0;
  readonly numberOfOutputs = 1;
  readonly channelCount = 2;
  readonly channelCountMode: ChannelCountMode = 'max';
  readonly channelInterpretation: ChannelInterpretation = 'speakers';

  readonly playbackRate: AudioParam<TContext, NativeContext>;
  readonly detune: AudioParam<TContext, NativeContext>;

  constructor(context: TContext) {
    this.context = context;

    // Dummy AudioParams
    this.playbackRate = new AudioParam(
      {
        defaultValue: 1,
        minValue: -3.4028234663852886e38,
        maxValue: 3.4028234663852886e38,
        value: 1,
        setValueAtTime: () => this.playbackRate,
        linearRampToValueAtTime: () => this.playbackRate,
        exponentialRampToValueAtTime: () => this.playbackRate,
        setTargetAtTime: () => this.playbackRate,
        setValueCurveAtTime: () => this.playbackRate,
        cancelScheduledValues: () => this.playbackRate,
        cancelAndHoldAtTime: () => this.playbackRate,
      },
      context
    );

    this.detune = new AudioParam(
      {
        defaultValue: 0,
        minValue: -3.4028234663852886e38,
        maxValue: 3.4028234663852886e38,
        value: 0,
        setValueAtTime: () => this.detune,
        linearRampToValueAtTime: () => this.detune,
        exponentialRampToValueAtTime: () => this.detune,
        setTargetAtTime: () => this.detune,
        setValueCurveAtTime: () => this.detune,
        cancelScheduledValues: () => this.detune,
        cancelAndHoldAtTime: () => this.detune,
      },
      context
    );
  }

  public enqueueBuffer(_buffer: AudioBuffer): string {
    availabilityWarn('AudioBufferQueueSourceNode.enqueueBuffer', 'web');
    return '';
  }

  public dequeueBuffer(_bufferId: string): void {
    availabilityWarn('AudioBufferQueueSourceNode.dequeueBuffer', 'web');
  }

  public clearBuffers(): void {
    availabilityWarn('AudioBufferQueueSourceNode.clearBuffers', 'web');
  }

  public start(_when: number = 0, _offset?: number): void {
    availabilityWarn('AudioBufferQueueSourceNode.start', 'web');
  }

  public pause(): void {
    availabilityWarn('AudioBufferQueueSourceNode.pause', 'web');
  }

  public stop(_when: number = 0): void {
    availabilityWarn('AudioBufferQueueSourceNode.stop', 'web');
  }

  public connect<ONode extends AudioNode<TContext, NativeContext>>(
    destination: ONode
  ): ONode;

  public connect(destination: AudioParam<TContext, NativeContext>): void;

  public connect<ONode extends AudioNode<TContext, NativeContext>>(
    destination: ONode | AudioParam<TContext, NativeContext>
  ): ONode | void {
    availabilityWarn('AudioBufferQueueSourceNode.connect', 'web');

    if (destination instanceof AudioParam) {
      return;
    }

    return destination;
  }

  public disconnect(): void;

  public disconnect<ONode extends AudioNode<TContext, NativeContext>>(
    destination: ONode
  ): void;

  public disconnect(destination: AudioParam<TContext, NativeContext>): void;

  public disconnect(
    _destination?:
      | AudioNode<TContext, NativeContext>
      | AudioParam<TContext, NativeContext>
  ): void {}

  public get onEnded(): OnEndedEventCallback | undefined {
    availabilityWarn('AudioBufferQueueSourceNode.onEnded', 'web');
    return undefined;
  }

  public set onEnded(_callback: OnEndedEventCallback | undefined) {
    availabilityWarn('AudioBufferQueueSourceNode.onEnded', 'web');
  }

  public get onPositionChangedInterval(): number {
    availabilityWarn(
      'AudioBufferQueueSourceNode.onPositionChangedInterval',
      'web'
    );
    return 0;
  }

  public set onPositionChangedInterval(_value: number) {
    availabilityWarn(
      'AudioBufferQueueSourceNode.onPositionChangedInterval',
      'web'
    );
  }

  public get onPositionChanged(): OnPositionChangedEventCallback | undefined {
    availabilityWarn('AudioBufferQueueSourceNode.onPositionChanged', 'web');
    return undefined;
  }

  public set onPositionChanged(
    _callback: OnPositionChangedEventCallback | null
  ) {
    availabilityWarn('AudioBufferQueueSourceNode.onPositionChanged', 'web');
  }
}
