import type {
  IGenericAudioDestinationNode,
  IGenericBaseAudioContext,
} from '../../types/generics';
import AudioNode from '../AudioNode';

export default class AudioDestinationNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
  >
  extends AudioNode<TContext, NContext, IGenericAudioDestinationNode<NContext>>
  implements IGenericAudioDestinationNode<TContext> {
  // TODO: implement on native side
  // readonly maxChannelCount: number;
}
