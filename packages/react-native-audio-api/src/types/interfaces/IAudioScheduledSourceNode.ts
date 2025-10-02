import type { OnEndedEventType } from '../../events/types';
import type { IGenericAudioNode, IGenericBaseAudioContext } from '../generics';

export type OnEndedEventCallback = (event: OnEndedEventType) => void;

export default interface IAudioScheduledSourceNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  start(when?: number): void;
  stop(when?: number): void;

  onEnded: OnEndedEventCallback | undefined;
}
