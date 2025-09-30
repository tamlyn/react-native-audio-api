export type ChannelCountMode = 'max' | 'clamped-max' | 'explicit';
export type ChannelInterpretation = 'speakers' | 'discrete';

export type BiquadFilterType =
  | 'lowpass'
  | 'highpass'
  | 'bandpass'
  | 'lowshelf'
  | 'highshelf'
  | 'peaking'
  | 'notch'
  | 'allpass';

export type WindowType = 'blackman' | 'hann';
