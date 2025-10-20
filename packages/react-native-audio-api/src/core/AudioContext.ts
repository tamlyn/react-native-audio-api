import BaseAudioContext, { IBaseAudioContext } from './BaseAudioContext';

import { createNativeContext } from './helpers';

interface Runtime {}

/**
 * The AudioContext interface represents an audio-processing graph built from
 * audio modules linked together, each represented by an AudioNode.
 */
export default class AudioContext<
  NativeContext extends IBaseAudioContext,
> extends BaseAudioContext<NativeContext> {
  private _audioRuntime: Runtime | null = null;

  constructor() {
    const context = createNativeContext();
    super(context as NativeContext);
  }
}

function testFunction() {
  const aCtx = new AudioContext();

  console.log(aCtx.sampleRate);
}
