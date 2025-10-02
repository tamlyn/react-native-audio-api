import type IGenericAudioParam from './IGenericAudioParam';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericAudioNode<
  TContext extends IGenericBaseAudioContext,
> {
  readonly context: TContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;

  connect<C extends IGenericBaseAudioContext, D extends IGenericAudioNode<C>>(
    destination: D
  ): D;
  connect(destination: IGenericAudioParam): void;

  disconnect<
    C extends IGenericBaseAudioContext,
    D extends IGenericAudioNode<C>,
  >(
    destination?: D
  ): void;
  disconnect(destination?: IGenericAudioParam): void;
}
