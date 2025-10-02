import type IAudioParam from './IAudioParam';
import type IAudioScheduledSourceNode from './IAudioScheduledSourceNode';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAudioBufferBaseSourceNode<
  TContext extends IBaseAudioContext,
> extends IAudioScheduledSourceNode<TContext> {
  detune: IAudioParam;
  playbackRate: IAudioParam;
}
