import type IAudioNode from './IAudioNode';
import type IAudioParam from './IAudioParam';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IGainNode<TContext extends IBaseAudioContext>
  extends IAudioNode<TContext> {
  readonly gain: IAudioParam;
}
