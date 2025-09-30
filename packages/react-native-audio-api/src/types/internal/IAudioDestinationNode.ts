import type IAudioNode from './IAudioNode';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAudioDestinationNode<
  TContext extends IBaseAudioContext,
> extends IAudioNode<TContext> {
  // TODO: implement on native side
  // readonly maxChannelCount: number;
}
