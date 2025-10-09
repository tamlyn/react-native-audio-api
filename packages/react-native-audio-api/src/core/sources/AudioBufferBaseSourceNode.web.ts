import type { IGenericBaseAudioContext } from '../../types/generics';
import type { IAudioBufferBaseSourceNode } from '../../types/interfaces';

/**
 * No implementation for web yet, shouldn't be used directly. Consider
 * implementing if/once we ship our own AudioBufferSourceNode implementation to
 * web.
 */
class AudioBufferBaseSourceNode {}

export default AudioBufferBaseSourceNode as unknown as IAudioBufferBaseSourceNode<IGenericBaseAudioContext>;
