import { IPeriodicWave } from '../interfaces';
import BaseAudioContext from './BaseAudioContext';
import { TPeriodicWaveOptions } from '../types';
import { PeriodicWaveConstraints } from '../defaults';
import { NotSupportedError } from '../errors';

export function validatePeriodicWaveOptions(
  sampleRate: number,
  options?: TPeriodicWaveOptions
): TPeriodicWaveOptions {
  let real: Float32Array | undefined;
  let imag: Float32Array | undefined;
  if (!options || (!options.real && !options.imag)) {
    // default to a sine wave
    if (sampleRate < 24000) {
      real = new Float32Array(2048);
      imag = new Float32Array(2048);
    } else if (sampleRate < 88200) {
      real = new Float32Array(4096);
      imag = new Float32Array(4096);
    } else {
      real = new Float32Array(16384);
      imag = new Float32Array(16384);
    }
    imag[1] = 1;
  } else {
    real = options?.real;
    imag = options?.imag;
    if (real && imag && real.length !== imag.length) {
      throw new NotSupportedError(
        "'real' and 'imag' arrays must have the same length"
      );
    }
    if (real && !imag) {
      imag = new Float32Array(real.length);
    } else if (!real && imag) {
      real = new Float32Array(imag.length);
    }
  }
  const norm: boolean = options?.disableNormalization
    ? options.disableNormalization
    : PeriodicWaveConstraints.disableNormalization!;
  return { real, imag, disableNormalization: norm };
}

export default class PeriodicWave {
  /** @internal */
  public readonly periodicWave: IPeriodicWave;

  constructor(context: BaseAudioContext, options?: TPeriodicWaveOptions) {
    const finalOptions = validatePeriodicWaveOptions(
      context.sampleRate,
      options
    );
    this.periodicWave = context.context.createPeriodicWave(
      finalOptions.real,
      finalOptions.imag,
      finalOptions.disableNormalization!
    );
  }
}
