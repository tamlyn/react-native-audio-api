import { TAudioNodeOptions, TGainOptions, TSteroPannerOptions } from './types';

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
