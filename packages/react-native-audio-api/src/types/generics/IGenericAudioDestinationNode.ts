import type IGenericAudioNode from './IGenericAudioNode';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IAudioDestinationNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  // TODO: implement on native side
  // readonly maxChannelCount: number;
}
