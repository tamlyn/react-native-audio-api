import type { IBaseAudioContext, IGainNode } from '../types/internal';
import AudioNode from './AudioNode';
import AudioParam from './AudioParam';

export default class GainNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioNode<TContext, NContext>
  implements IGainNode<TContext>
{
  readonly gain: AudioParam<TContext, NContext>;

  constructor(context: TContext, gain: IGainNode<NContext>) {
    super(context, gain);
    this.gain = new AudioParam<TContext, NContext>(gain.gain, context);
  }
}
