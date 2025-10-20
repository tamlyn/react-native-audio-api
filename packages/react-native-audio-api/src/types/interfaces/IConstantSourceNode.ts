import { IGenericAudioParam, IGenericBaseAudioContext } from '../generics';
import IAudioScheduledSourceNode from './IAudioScheduledSourceNode';

export default interface IConstantSourceNode<
  TContext extends IGenericBaseAudioContext,
> extends IAudioScheduledSourceNode<TContext> {
  readonly offset: IGenericAudioParam<TContext>;
}
