import { InvalidStateError, RangeError } from '../../errors';
// import { EventEmptyType } from '../../events';
import type {
  IGenericAudioBuffer,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type { IAudioBufferSourceNode } from '../../types/interfaces';
import AudioBuffer from '../AudioBuffer';
import AudioBufferBaseSourceNode, {
  NativeAudioBufferBaseSourceNode,
} from './AudioBufferBaseSourceNode.native';

interface NativeAudioBufferSourceNode extends NativeAudioBufferBaseSourceNode {
  readonly buffer: IGenericAudioBuffer | null;
  loopSkip: boolean;
  loop: boolean;
  loopStart: number;
  loopEnd: number;

  setBuffer(buffer: IGenericAudioBuffer | null): void;

  start(when?: number): void;
  start(when: number, offset: number, duration?: number): void;
}

export default class AudioBufferSourceNode<
    TContext extends IGenericBaseAudioContext,
  >
  extends AudioBufferBaseSourceNode<TContext, NativeAudioBufferSourceNode>
  implements IAudioBufferSourceNode<TContext>
{
  public get buffer(): AudioBuffer | null {
    if (!this.node.buffer) {
      return null;
    }

    return new AudioBuffer(this.node.buffer);
  }

  public set buffer(buffer: IGenericAudioBuffer | null) {
    this.node.setBuffer(buffer);
  }

  public get loopSkip(): boolean {
    return this.node.loopSkip;
  }

  public set loopSkip(value: boolean) {
    this.node.loopSkip = value;
  }

  public get loop(): boolean {
    return this.node.loop;
  }

  public set loop(value: boolean) {
    this.node.loop = value;
  }

  public get loopStart(): number {
    return this.node.loopStart;
  }

  public set loopStart(value: number) {
    this.node.loopStart = value;
  }

  public get loopEnd(): number {
    return this.node.loopEnd;
  }

  public set loopEnd(value: number) {
    this.node.loopEnd = value;
  }

  public start(when: number = 0, offset: number = 0, duration?: number): void {
    if (when < 0) {
      throw new RangeError(
        `when must be a finite non-negative number: ${when}`
      );
    }

    if (offset < 0) {
      throw new RangeError(
        `offset must be a finite non-negative number: ${offset}`
      );
    }

    if (duration && duration < 0) {
      throw new RangeError(
        `duration must be a finite non-negative number: ${duration}`
      );
    }

    if (this.hasBeenStarted) {
      throw new InvalidStateError('Cannot call start more than once');
    }

    this.hasBeenStarted = true;
    this.node.start(when, offset, duration);
  }

  // ?????????????????????????
  // TODO: Generic this out?
  // public override get onEnded(): ((event: EventEmptyType) => void) | undefined {
  //   return super.onEnded as ((event: EventEmptyType) => void) | undefined;
  // }
  // public override set onEnded(
  //   callback: ((event: EventEmptyType) => void) | null
  // ) {
  //   super.onEnded = callback;
  // }
}
