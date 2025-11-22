import {
  AudioContext,
  GainNode,
  BiquadFilterNode,
  ConvolverNode,
  WaveShaperNode,
  AudioNode,
} from 'react-native-audio-api';

export function createGainEffect(
  audioContext: AudioContext,
  gain: number = 1.0
): GainNode {
  const gainNode = audioContext.createGain();
  gainNode.gain.setValueAtTime(gain, audioContext.currentTime);
  return gainNode;
}

export function createLowPassFilter(
  audioContext: AudioContext,
  frequency: number = 1000
): BiquadFilterNode {
  const filter = audioContext.createBiquadFilter();
  filter.type = 'lowpass';
  filter.frequency.setValueAtTime(frequency, audioContext.currentTime);
  filter.Q.setValueAtTime(1, audioContext.currentTime);
  return filter;
}

export function createHighPassFilter(
  audioContext: AudioContext,
  frequency: number = 300
): BiquadFilterNode {
  const filter = audioContext.createBiquadFilter();
  filter.type = 'highpass';
  filter.frequency.setValueAtTime(frequency, audioContext.currentTime);
  filter.Q.setValueAtTime(1, audioContext.currentTime);
  return filter;
}

export function createSimpleReverb(
  audioContext: AudioContext,
  roomSize: number = 0.5,
  decayTime: number = 2
): ConvolverNode {
  const convolver = audioContext.createConvolver();

  const sampleRate = audioContext.sampleRate;
  const length = sampleRate * decayTime;
  const impulse = audioContext.createBuffer(2, length, sampleRate);

  for (let channel = 0; channel < 2; channel++) {
    const channelData = impulse.getChannelData(channel);
    for (let i = 0; i < length; i++) {
      const decay = Math.pow(1 - i / length, 2);
      channelData[i] = (Math.random() * 2 - 1) * decay * roomSize;
    }
  }

  convolver.buffer = impulse;
  return convolver;
}

export function createOverdrive(
  audioContext: AudioContext,
  amount: number = 0.5
): WaveShaperNode {
  const waveShaper = audioContext.createWaveShaper();

  const samples = 44100;
  const curve = new Float32Array(samples);

  const clampedAmount = Math.max(0, Math.min(1, amount));
  const drive = 2 + clampedAmount * 30;

  for (let i = 0; i < samples; i++) {
    const x = (i * 2) / samples - 1;
    const driven = x * drive;

    let distorted;
    if (driven > 0) {
      distorted = Math.tanh(driven * 1.5) * 0.8;
    } else {
      distorted = Math.tanh(driven * 1.2) * 0.9;
    }

    const harmonics = Math.sin(driven * 3) * 0.1 * clampedAmount;

    curve[i] = Math.max(-1, Math.min(1, distorted + harmonics)) * 3;
  }

  waveShaper.curve = curve;
  waveShaper.oversample = '4x';

  return waveShaper;
}

export function createBandPassFilter(
  audioContext: AudioContext,
  lowFreq: number = 800,
  highFreq: number = 3000
): BiquadFilterNode {
  const filter = audioContext.createBiquadFilter();
  filter.type = 'bandpass';

  const centerFreq = Math.sqrt(lowFreq * highFreq);
  filter.frequency.setValueAtTime(centerFreq, audioContext.currentTime);

  const Q = centerFreq / (highFreq - lowFreq);
  filter.Q.setValueAtTime(Q, audioContext.currentTime);

  return filter;
}

export function createEffectsMap(
  effects: { name: string; node: AudioNode }[]
): Map<string, AudioNode> {
  const effectsMap = new Map<string, AudioNode>();

  effects.forEach(({ name, node }) => {
    effectsMap.set(name, node);
  });

  return effectsMap;
}

export const presetEffects = {
  ambient: (audioContext: AudioContext) =>
    createEffectsMap([
      { name: 'reverb', node: createSimpleReverb(audioContext, 0.3, 1.5) },
      { name: 'lowpass', node: createLowPassFilter(audioContext, 8000) },
    ]),

  warm: (audioContext: AudioContext) =>
    createEffectsMap([
      { name: 'lowpass', node: createLowPassFilter(audioContext, 3000) },
      { name: 'gain', node: createGainEffect(audioContext, 1.2) },
    ]),

  distorted: (audioContext: AudioContext) =>
    createEffectsMap([
      { name: 'overdrive', node: createOverdrive(audioContext, 0.85) },
      { name: 'midpass', node: createBandPassFilter(audioContext, 800, 3000) },
      { name: 'lowpass', node: createLowPassFilter(audioContext, 6000) },
      { name: 'gain', node: createGainEffect(audioContext, 0.4) },
    ]),

  test: (audioContext: AudioContext) =>
    createEffectsMap([
      { name: 'gain', node: createGainEffect(audioContext, 0.3) },
    ]),

  overdrive_only: (audioContext: AudioContext) =>
    createEffectsMap([
      { name: 'overdrive', node: createOverdrive(audioContext, 0.9) },
    ]),
};
