import type { IBaseAudioContext } from '../BaseAudioContext';

interface INativeAudioContext extends IBaseAudioContext {
  bebebe: string;
  hakoonaMatata: () => void;
}

declare global {
  function createAudioContext(): INativeAudioContext;
}

export default function createNativeContextMobile(): IBaseAudioContext {
  return global.createAudioContext();
}
