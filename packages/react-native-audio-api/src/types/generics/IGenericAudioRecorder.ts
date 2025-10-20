import type IGenericBaseAudioContext from './IGenericBaseAudioContext';
import type IGenericRecorderAdapterNode from './IGenericRecorderAdapterNode';

export default interface IGenericAudioRecorder<
  TContext extends IGenericBaseAudioContext,
> {
  start: () => void;
  stop: () => void;
  // TODO: fix this TS error
  connect: (node: IGenericRecorderAdapterNode<TContext>) => void;
  disconnect: () => void;

  // TODO: onAudioReady
}
