import type IAudioParam from './IAudioParam';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAudioNode<TContext extends IBaseAudioContext> {
  readonly context: TContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;

  connect<C extends IBaseAudioContext, D extends IAudioNode<C>>(
    destination: D
  ): D;
  connect(destination: IAudioParam): void;

  disconnect<C extends IBaseAudioContext, D extends IAudioNode<C>>(
    destination?: D
  ): void;
  disconnect(destination?: IAudioParam): void;
}
