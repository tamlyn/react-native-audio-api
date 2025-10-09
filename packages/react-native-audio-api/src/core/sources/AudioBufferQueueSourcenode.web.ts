import type { IGenericBaseAudioContext } from '../../types/generics';
import type { IAudioBufferQueueSourceNode } from '../../types/interfaces';

class AudioBufferQueueSourceNode {}

// Maybe someday we will implement this for the web as well
export default AudioBufferQueueSourceNode as unknown as IAudioBufferQueueSourceNode<IGenericBaseAudioContext>;
