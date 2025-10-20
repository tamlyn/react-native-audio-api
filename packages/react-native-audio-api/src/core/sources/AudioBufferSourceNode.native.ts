import { InvalidStateError, RangeError } from '../../errors';
import { AudioEventSubscription } from '../../events';
import type {
  IGenericAudioBuffer,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type {
  IAudioBufferSourceNode,
  LoopEndedEventCallback,
} from '../../types/interfaces';
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

  onLoopEnded: string;
}

export default class AudioBufferSourceNode<
    TContext extends IGenericBaseAudioContext,
  >
  extends AudioBufferBaseSourceNode<TContext, NativeAudioBufferSourceNode>
  implements IAudioBufferSourceNode<TContext>
{
  private mBuffer: AudioBuffer | null = null;
  private onLoopEndedSubscription?: AudioEventSubscription;
  private onLoopEndedCallback?: LoopEndedEventCallback;

  public get buffer(): AudioBuffer | null {
    return this.mBuffer;
  }

  public set buffer(buffer: IGenericAudioBuffer | null) {
    this.node.setBuffer(buffer);

    if (!this.node.buffer) {
      this.mBuffer = null;
      return;
    }

    this.mBuffer = new AudioBuffer(this.node.buffer);
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

  public get onLoopEnded(): LoopEndedEventCallback | undefined {
    return this.onLoopEndedCallback;
  }

  public set onLoopEnded(callback: LoopEndedEventCallback | null) {
    if (!callback) {
      this.node.onLoopEnded = '0';
      this.onLoopEndedSubscription?.remove();
      this.onLoopEndedSubscription = undefined;
      this.onLoopEndedCallback = undefined;

      return;
    }

    this.onLoopEndedCallback = callback;
    this.onLoopEndedSubscription = this.audioEventEmitter.addAudioEventListener(
      'loopEnded',
      callback
    );

    this.node.onLoopEnded = this.onLoopEndedSubscription.subscriptionId;
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
