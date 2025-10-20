import type { IBaseAudioContext } from '../BaseAudioContext';

export default function createNativeContextWeb(): IBaseAudioContext {
  return new window.AudioContext() as IBaseAudioContext;
}
