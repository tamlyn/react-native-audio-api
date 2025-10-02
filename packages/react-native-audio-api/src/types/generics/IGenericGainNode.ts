import type IGenericAudioNode from './IGenericAudioNode';
import type IGenericAudioParam from './IGenericAudioParam';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericGainNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  readonly gain: IGenericAudioParam<TContext>;
}
