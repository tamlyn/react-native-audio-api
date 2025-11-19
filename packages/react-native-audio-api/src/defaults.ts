import {
  TAudioNodeOptions,
  TGainOptions,
  TSteroPannerOptions,
  TConvolverOptions,
  TConstantSourceOptions,
  TPeriodicWaveConstraints,
} from './types';

export const AudioNodeOptions: TAudioNodeOptions = {
  channelCount: 2,
  channelCountMode: 'max',
  channelInterpretation: 'speakers',
};

export const GainOptions: TGainOptions = {
  ...AudioNodeOptions,
  gain: 1,
};

export const SteroPannerOptions: TSteroPannerOptions = {
  ...AudioNodeOptions,
  channelCountMode: 'clamped-max',
  pan: 0,
};

export const ConvolverOptions: TConvolverOptions = {
  ...AudioNodeOptions,
  disableNormalization: false,
};

export const ConstantSourceOptions: TConstantSourceOptions = {
  offset: 1,
};

export const PeriodicWaveConstraints: TPeriodicWaveConstraints = {
  disableNormalization: false,
};
