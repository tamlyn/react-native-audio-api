import type {
  IGenericBaseAudioContext,
  IGenericPeriodicWave,
} from '../types/generics';

export default class PeriodicWave<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
> implements IGenericPeriodicWave<TContext>
{
  /** @internal */
  public readonly periodicWave: IGenericPeriodicWave<NContext>;

  constructor(periodicWave: IGenericPeriodicWave<NContext>) {
    this.periodicWave = periodicWave;
  }
}
