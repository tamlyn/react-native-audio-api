import { InvalidStateError, RangeError } from '../errors';
import type { OnEndedEventType } from '../events/types';
import type {
  IAudioScheduledSourceNode,
  IBaseAudioContext,
} from '../types/internal';
import AudioNode from './AudioNode';

export type OnEndedEventCallback = (event: OnEndedEventType) => void;

export default class AudioScheduledSourceNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioNode<TContext, NContext>
  implements IAudioScheduledSourceNode<TContext>
{
  protected hasBeenStarted: boolean = false;
  private onEndedCallback: OnEndedEventCallback | null = null;

  public start(when: number = 0): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    if (this.hasBeenStarted) {
      throw new InvalidStateError('Cannot call start more than once');
    }

    this.hasBeenStarted = true;
    (this.node as IAudioScheduledSourceNode<NContext>).start(when);
  }

  public stop(when: number = 0): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    if (!this.hasBeenStarted) {
      throw new InvalidStateError(
        'Cannot call stop without calling start first'
      );
    }

    (this.node as IAudioScheduledSourceNode<NContext>).stop(when);
  }

  public get onEnded(): OnEndedEventCallback | undefined {
    return this.onEndedCallback ?? undefined;
  }

  public set onEnded(callback: OnEndedEventCallback | null) {
    if (!callback) {
      this.onEndedCallback = null;
      (this.node as unknown as globalThis.AudioScheduledSourceNode).onended =
        null;

      return;
    }

    this.onEndedCallback = callback;
    (this.node as unknown as globalThis.AudioScheduledSourceNode).onended =
      () => {
        this.onEndedCallback?.({ bufferId: undefined });
      };
  }
}
