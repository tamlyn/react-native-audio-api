import { ChannelCountMode, ChannelInterpretation } from '../types';
import type { IAudioNode, IBaseAudioContext } from '../types/internal';
import AudioParam from './AudioParam';

export default class AudioNode<
  TContext extends IBaseAudioContext,
  NContext extends IBaseAudioContext,
> implements IAudioNode<TContext>
{
  readonly context: TContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;

  protected readonly node: IAudioNode<NContext>;

  constructor(context: TContext, node: IAudioNode<NContext>) {
    this.context = context;
    this.node = node;
    this.numberOfInputs = this.node.numberOfInputs;
    this.numberOfOutputs = this.node.numberOfOutputs;
    this.channelCount = this.node.channelCount;
    this.channelCountMode = this.node.channelCountMode;
    this.channelInterpretation = this.node.channelInterpretation;
  }

  public connect(
    destination: AudioNode<TContext, NContext>
  ): AudioNode<TContext, NContext>;

  public connect(destination: AudioParam<TContext, NContext>): void;
  public connect(
    destination: AudioNode<TContext, NContext> | AudioParam<TContext, NContext>
  ): AudioNode<TContext, NContext> | void {
    if (this.context !== destination.context) {
      throw new Error(
        'Source and destination are from different BaseAudioContexts'
      );
    }

    if (destination instanceof AudioParam) {
      this.node.connect(destination.param);
      return;
    }

    this.node.connect(destination.node);
    return destination;
  }

  public disconnect(): void;
  public disconnect(destination: AudioNode<TContext, NContext>): void;
  public disconnect(destination: AudioParam<TContext, NContext>): void;
  public disconnect(
    destination?: AudioNode<TContext, NContext> | AudioParam<TContext, NContext>
  ): void {
    if (destination instanceof AudioParam) {
      this.node.disconnect(destination.param);
      return;
    }

    this.node.disconnect(destination?.node);
  }
}
