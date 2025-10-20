import type { EventEmptyType } from '../../events';
import type { IGenericBaseAudioContext } from '../generics';
import type IAudioBufferBaseSourceNode from './IAudioBufferBaseSourceNode';

export type LoopEndedEvent = EventEmptyType;
export type LoopEndedEventCallback = (event: LoopEndedEvent) => void;

export default interface IAudioBufferSourceNode<
  TContext extends IGenericBaseAudioContext,
> extends IAudioBufferBaseSourceNode<TContext> {
  buffer: AudioBuffer | null;
  loopSkip: boolean;
  loop: boolean;
  loopStart: number;
  loopEnd: number;

  start(when?: number, offset?: number, duration?: number): void;

  onLoopEnded: LoopEndedEventCallback | undefined;
}
