import type { BiquadFilterType } from '../properties';
import type IAudioNode from './IAudioNode';
import type IAudioParam from './IAudioParam';
import type IBaseAudioContext from './IBaseAudioContext';

export default interface IStereoPannerNode<TContext extends IBaseAudioContext>
  extends IAudioNode<TContext> {
  readonly frequency: IAudioParam;
  readonly detune: IAudioParam;
  readonly Q: IAudioParam;
  readonly gain: IAudioParam;
  type: BiquadFilterType;

  getFrequencyResponse(
    frequencyArray: Float32Array,
    magResponseOutput: Float32Array,
    phaseResponseOutput: Float32Array
  ): void;
}
