import { InvalidStateError, RangeError } from '../../errors';
import type {
  IGenericAudioNode,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type {
  IAudioScheduledSourceNode,
  OnEndedEventCallback,
} from '../../types/interfaces';
import AudioNode from '../AudioNode';

export interface IAbstractNativeAudioScheduledSourceNode<
  NContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<NContext> {
  start(when?: number): void;
  stop(when?: number): void;
}

export default abstract class BaseAudioScheduledSourceNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
    NNode extends
      IAbstractNativeAudioScheduledSourceNode<NContext> = IAbstractNativeAudioScheduledSourceNode<NContext>,
  >
  extends AudioNode<TContext, NContext, NNode>
  implements IAudioScheduledSourceNode<TContext>
{
  protected hasBeenStarted: boolean = false;

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
    this.node.start(when);
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

    this.node.stop(when);
  }

  public abstract get onEnded(): OnEndedEventCallback | undefined;
  public abstract set onEnded(callback: OnEndedEventCallback | null);
}
