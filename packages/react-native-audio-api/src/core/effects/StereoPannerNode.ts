import type {
  IGenericBaseAudioContext,
  IGenericStereoPannerNode,
} from '../../types/generics';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

export default class StereoPannerNode<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
> extends AudioNode<TContext, NContext, IGenericStereoPannerNode<NContext>> {
  readonly pan: AudioParam<TContext, NContext>;

  constructor(context: TContext, pan: IGenericStereoPannerNode<NContext>) {
    super(context, pan);
    this.pan = new AudioParam<TContext, NContext>(pan.pan, context);
  }
}
