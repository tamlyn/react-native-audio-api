import type { IGenericAudioNode, IGenericBaseAudioContext } from '../generics';
import type { WindowType } from '../properties';

export default interface IAnalyserNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  fftSize: number;
  readonly frequencyBinCount: number;
  minDecibels: number;
  maxDecibels: number;
  smoothingTimeConstant: number;
  window: WindowType;

  getByteFrequencyData: (array: Uint8Array) => void;
  getByteTimeDomainData: (array: Uint8Array) => void;

  getFloatFrequencyData: (array: Float32Array) => void;
  getFloatTimeDomainData: (array: Float32Array) => void;
}
