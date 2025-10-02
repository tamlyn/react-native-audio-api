import IGenericBaseAudioContext from './IGenericBaseAudioContext';

/**
 * TContext is needed to ensure that we won't try to mix objects from different
 * contexts (especially different layers of contexts)
 */
export default interface IGenericPeriodicWave<
  // eslint-disable-next-line @typescript-eslint/no-unused-vars
  TContext extends IGenericBaseAudioContext,
> {}
