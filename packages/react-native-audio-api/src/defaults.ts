import { TAudioNodeOptions, TGainOptions } from './types';

export const AudioNodeOptions: TAudioNodeOptions = {
  channelCount: 2,
  channelCountMode: 'max',
  channelInterpretation: 'speakers',
};

export const GainOptions: TGainOptions = {
  ...AudioNodeOptions,
  gain: 1,
};
