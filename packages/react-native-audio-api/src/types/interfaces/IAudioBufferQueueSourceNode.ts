import { IGenericAudioBuffer, IGenericBaseAudioContext } from '../generics';
import IAudioBufferBaseSourceNode from './IAudioBufferBaseSourceNode';

export default interface IAudioBufferQueueSourceNode<
  TContext extends IGenericBaseAudioContext,
  TAudioBuffer extends IGenericAudioBuffer = IGenericAudioBuffer,
> extends IAudioBufferBaseSourceNode<TContext> {
  dequeueBuffer: (bufferId: string) => void;
  clearBuffers: () => void;

  enqueueBuffer: (audioBuffer: TAudioBuffer) => string;
  pause: () => void;
}
