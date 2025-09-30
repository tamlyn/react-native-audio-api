import type {
  IAudioDestinationNode,
  IBaseAudioContext,
} from '../../types/internal';
import AudioNode from '../AudioNode';

export default class AudioDestinationNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioNode<TContext, NContext>
  implements IAudioDestinationNode<TContext> {
  // TODO: implement on native side
  // readonly maxChannelCount: number;
}
