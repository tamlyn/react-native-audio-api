import type { BiquadFilterType } from '../properties';
import type IGenericAudioNode from './IGenericAudioNode';
import type IGenericAudioParam from './IGenericAudioParam';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericBiquadFilterNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  readonly frequency: IGenericAudioParam<TContext>;
  readonly detune: IGenericAudioParam<TContext>;
  readonly Q: IGenericAudioParam<TContext>;
  readonly gain: IGenericAudioParam<TContext>;
  type: BiquadFilterType;

  getFrequencyResponse(
    frequencyArray: Float32Array,
    magResponseOutput: Float32Array,
    phaseResponseOutput: Float32Array
  ): void;
}
