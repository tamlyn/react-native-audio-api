import { OscillatorType } from '../../types/properties';

import type IAudioParam from './IAudioParam';
import type IAudioScheduledSourceNode from './IAudioScheduledSourceNode';
import type IBaseAudioContext from './IBaseAudioContext';
import type IPeriodicWave from './IPeriodicWave';

export default interface IOscillatorNode<TContext extends IBaseAudioContext>
  extends IAudioScheduledSourceNode<TContext> {
  readonly frequency: IAudioParam;
  readonly detune: IAudioParam;
  type: OscillatorType;

  setPeriodicWave(periodicWave: IPeriodicWave): void;
}
