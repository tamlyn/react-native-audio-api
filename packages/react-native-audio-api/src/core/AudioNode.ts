import { ChannelCountMode, ChannelInterpretation } from '../types';
import type {
  IGenericAudioNode,
  IGenericBaseAudioContext,
} from '../types/generics';
import AudioParam from './AudioParam';

export default class AudioNode<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
  TNode extends IGenericAudioNode<NContext> = IGenericAudioNode<NContext>,
> implements IGenericAudioNode<TContext>
{
  readonly context: TContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;

  protected readonly node: TNode;

  constructor(context: TContext, node: TNode) {
    this.context = context;
    this.node = node;
    this.numberOfInputs = this.node.numberOfInputs;
    this.numberOfOutputs = this.node.numberOfOutputs;
    this.channelCount = this.node.channelCount;
    this.channelCountMode = this.node.channelCountMode;
    this.channelInterpretation = this.node.channelInterpretation;
  }

  public connect<ONode extends AudioNode<TContext, NContext>>(
    destination: ONode
  ): ONode;

  public connect(destination: AudioParam<TContext, NContext>): void;

  public connect<ONode extends AudioNode<TContext, NContext>>(
    destination: ONode | AudioParam<TContext, NContext>
  ): ONode | void {
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

  public disconnect<ONode extends AudioNode<TContext, NContext>>(
    destination: ONode
  ): void;

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
