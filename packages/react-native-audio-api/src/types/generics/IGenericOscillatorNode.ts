import { OscillatorType } from '../../types/properties';
import type IGenericAudioNode from './IGenericAudioNode';
import type IGenericAudioParam from './IGenericAudioParam';
import type IGenericBaseAudioContext from './IGenericBaseAudioContext';
import type IGenericPeriodicWave from './IGenericPeriodicWave';

/**
 * TODO: TBD/FIXME - OscillatorNode is implemented as generic interface, but in
 * reality it inherits from AudioScheduledSourceNode which can't be a generic
 * interface as implementation differs between platforms (onEnded vs onended).
 * This means we will have to possibly remember to provide correct types for
 * onEnded method? Which possibly could be written better in the future. For
 * now, lets leave it as it is, its just an oscillator lol and writing this
 * comment is already too much time spent here. :cheers:
 */
export default interface IGenericOscillatorNode<
  TContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<TContext> {
  readonly frequency: IGenericAudioParam<TContext>;
  readonly detune: IGenericAudioParam<TContext>;
  type: OscillatorType;

  setPeriodicWave(periodicWave: IGenericPeriodicWave<TContext>): void;
}
