import type { WindowType } from '../properties';
import type IAudioNode from './IAudioNode';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IAnalyserNode<TContext extends IBaseAudioContext>
  extends IAudioNode<TContext> {
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
