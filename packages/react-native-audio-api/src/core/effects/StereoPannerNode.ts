import type {
  IBaseAudioContext,
  IStereoPannerNode,
} from '../../types/internal';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

export default class StereoPannerNode<
  TContext extends IBaseAudioContext,
  NContext extends IBaseAudioContext,
> extends AudioNode<TContext, NContext> {
  readonly pan: AudioParam<TContext, NContext>;

  constructor(context: TContext, pan: IStereoPannerNode<NContext>) {
    super(context, pan);
    this.pan = new AudioParam<TContext, NContext>(pan.pan, context);
  }
}
