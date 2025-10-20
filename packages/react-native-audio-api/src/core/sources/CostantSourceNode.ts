import { IGenericBaseAudioContext } from '../../types/generics';
import IConstantSourceNode from '../../types/interfaces/IConstantSourceNode';
import AudioParam from '../AudioParam';
import AudioScheduledSourceNode from './AudioScheduledSourceNode';

export default class ConstantSourceNode<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
> extends AudioScheduledSourceNode<TContext, IConstantSourceNode<NContext>> {
  readonly offset: AudioParam<TContext, NContext>;

  constructor(context: TContext, node: IConstantSourceNode<NContext>) {
    super(context, node);

    this.offset = new AudioParam(node.offset, context);
  }
}
