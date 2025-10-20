import type {
  IGenericBaseAudioContext,
  IGenericRecorderAdapterNode,
} from '../../types/generics';
import AudioNode from '../AudioNode';

export default class RecorderAdapterNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
  >
  extends AudioNode<TContext, NContext>
  implements IGenericRecorderAdapterNode<TContext>
{
  /** @internal */
  public wasConnected: boolean = false;

  /** @internal */
  public getNode(): IGenericRecorderAdapterNode<NContext> {
    return this.node;
  }
}
