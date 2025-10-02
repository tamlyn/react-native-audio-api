import type IAudioBuffer from './IAudioBuffer';
import type IAudioBufferBaseSourceNode from './IAudioBufferBaseSourceNode';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAudioBufferSourceNode<
  TContext extends IBaseAudioContext,
> extends IAudioBufferBaseSourceNode<TContext> {
  buffer: IAudioBuffer | null;
  loop: boolean;
  loopSkip: boolean;
  loopStart: number;
  loopEnd: number;

  start: (when?: number, offset?: number, duration?: number) => void;
}
