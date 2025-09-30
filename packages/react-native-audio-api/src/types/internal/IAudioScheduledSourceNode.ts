import type IAudioNode from './IAudioNode';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAudioScheduledSourceNode<
  TContext extends IBaseAudioContext,
> extends IAudioNode<TContext> {
  start(when?: number): void;
  stop(when?: number): void;

  // TODO: what to do about onEnded=string(native) onEnded=function(TS) and onended=function(web)
}
