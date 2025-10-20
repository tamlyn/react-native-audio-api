import IGenericAudioNode from './IGenericAudioNode';
import IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericRecorderAdapterNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {}
