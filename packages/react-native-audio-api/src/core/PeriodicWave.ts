import type { IPeriodicWave } from '../types/internal';

export default class PeriodicWave {
  /** @internal */
  public readonly periodicWave: IPeriodicWave;

  constructor(periodicWave: IPeriodicWave) {
    this.periodicWave = periodicWave;
  }
}
