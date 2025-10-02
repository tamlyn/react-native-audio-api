import type {
  IGenericBaseAudioContext,
  IGenericGainNode,
} from '../../types/generics';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

export default class GainNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
  >
  extends AudioNode<TContext, NContext, IGenericGainNode<NContext>>
  implements IGenericGainNode<TContext>
{
  readonly gain: AudioParam<TContext, NContext>;

  constructor(context: TContext, gain: IGenericGainNode<NContext>) {
    super(context, gain);
    this.gain = new AudioParam<TContext, NContext>(gain.gain, context);
  }
}
