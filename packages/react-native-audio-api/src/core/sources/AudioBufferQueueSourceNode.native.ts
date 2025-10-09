import { RangeError } from '../../errors';
import type {
  IGenericAudioBuffer,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type { IAudioBufferQueueSourceNode } from '../../types/interfaces';
import AudioBuffer from '../AudioBuffer';
import AudioBufferBaseSourceNode, {
  NativeAudioBufferBaseSourceNode,
} from './AudioBufferBaseSourceNode.native';

interface NativeAudioBufferQueueSourceNode
  extends NativeAudioBufferBaseSourceNode {
  enqueueBuffer(buffer: IGenericAudioBuffer): string;
  dequeueBuffer(bufferId: number): void;
  clearBuffers(): void;

  start(when?: number): void;
  pause(): void;
  stop(when?: number): void;
}

export default class AudioBufferQueueSourceNode<
    TContext extends IGenericBaseAudioContext,
  >
  extends AudioBufferBaseSourceNode<TContext, NativeAudioBufferQueueSourceNode>
  implements IAudioBufferQueueSourceNode<TContext, AudioBuffer>
{
  public enqueueBuffer(buffer: AudioBuffer): string {
    return this.node.enqueueBuffer(buffer.buffer);
  }

  public dequeueBuffer(bufferId: string): void {
    const id = parseInt(bufferId, 10);

    if (isNaN(id) || id < 0) {
      throw new RangeError(
        `bufferId must be a non-negative integer: ${bufferId}`
      );
    }

    this.node.dequeueBuffer(id);
  }

  public clearBuffers(): void {
    this.node.clearBuffers();
  }

  public override start(when: number = 0, _offset?: number): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    this.node.start(when);
  }

  public override stop(when: number = 0): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    this.node.stop(when);
  }

  public pause(): void {
    this.node.pause();
  }
}
