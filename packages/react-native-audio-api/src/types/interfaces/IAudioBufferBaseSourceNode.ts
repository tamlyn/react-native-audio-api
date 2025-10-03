import type { EventTypeWithValue } from '../../events';
import type { IGenericAudioParam, IGenericBaseAudioContext } from '../generics';
import type IAudioScheduledSourceNode from './IAudioScheduledSourceNode';

export type OnPositionChangedEventCallback = (
  event: EventTypeWithValue
) => void;

export default interface IAudioBufferBaseSourceNode<
  TContext extends IGenericBaseAudioContext,
> extends IAudioScheduledSourceNode<TContext> {
  readonly playbackRate: IGenericAudioParam<TContext>;
  readonly detune: IGenericAudioParam<TContext>;

  onPositionChanged: OnPositionChangedEventCallback | undefined;
  onPositionChangedInterval: number;
}
