import type IGenericAudioNode from './IGenericAudioNode';
import type IGenericAudioParam from './IGenericAudioParam';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericStereoPannerNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  readonly pan: IGenericAudioParam<TContext>;
}
