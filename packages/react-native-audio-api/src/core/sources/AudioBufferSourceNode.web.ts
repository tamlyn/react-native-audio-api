import { InvalidStateError, RangeError } from '../../errors';

import type {
  IGenericAudioBuffer,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type { IAudioBufferSourceNode } from '../../types/interfaces';
import { ActionQueue } from '../../utils';
import AudioBuffer from '../AudioBuffer';
import AudioParam from '../AudioParam';

type NativeAudioContext = globalThis.BaseAudioContext;
type NativeAudioBufferSourceNode = globalThis.AudioBufferSourceNode;

type WebBufferSourceNode = NativeAudioBufferSourceNode;

// Note: this class do not extend from AudioScheduledSourceNode or AudioNode
// because of the promise based access nature of the stretcher node
export default class AudioBufferSourceNode<
  TContext extends IGenericBaseAudioContext,
  NNode extends WebBufferSourceNode = NativeAudioBufferSourceNode,
> implements IAudioBufferSourceNode<TContext>
{
  readonly playbackRate: AudioParam<TContext, NativeAudioContext>;
  readonly detune: AudioParam<TContext, NativeAudioContext>;

  private mPitchCorrection: boolean;

  private mLoop: boolean = false;
  private mLoopStart: number = 0;
  private mLoopEnd: number = 0;

  private mBuffer: AudioBuffer | null = null;

  private actionQueue = new ActionQueue();

  private hasBeenStarted = false;

  constructor(
    context: TContext,
    nodePromise: Promise<NNode>,
    pitchCorrection: boolean
  ) {
    this.mPitchCorrection = pitchCorrection;

    if (!pitchCorrection) {
      this.playbackRate = new AudioParam(null, context);
      this.detune = new AudioParam(null, context);
    }
  }
}
