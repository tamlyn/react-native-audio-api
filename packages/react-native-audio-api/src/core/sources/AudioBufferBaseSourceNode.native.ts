import { AudioEventSubscription } from '../../events';
import type {
  IGenericAudioParam,
  IGenericBaseAudioContext,
} from '../../types/generics';
import {
  IAudioBufferBaseSourceNode,
  OnPositionChangedEventCallback,
} from '../../types/interfaces';
import AudioParam from '../AudioParam';
import AudioScheduledSourceNode, {
  MobileAudioScheduledSourceNode,
} from './AudioScheduledSourceNode.native';

interface NativeAudioContext {}

export interface NativeAudioBufferBaseSourceNode
  extends MobileAudioScheduledSourceNode {
  readonly playbackRate: IGenericAudioParam<NativeAudioContext>;
  readonly detune: IGenericAudioParam<NativeAudioContext>;

  onPositionChanged: string; // subscriptionId or '0' for none
  onPositionChangedInterval: number;
}

export default class AudioBufferBaseSourceNode<
    TContext extends IGenericBaseAudioContext,
    NNode extends
      NativeAudioBufferBaseSourceNode = NativeAudioBufferBaseSourceNode,
  >
  extends AudioScheduledSourceNode<TContext, NNode>
  implements IAudioBufferBaseSourceNode<TContext>
{
  readonly playbackRate: AudioParam<TContext, NativeAudioContext>;
  readonly detune: AudioParam<TContext, NativeAudioContext>;
  private positionChangedSubscription?: AudioEventSubscription;
  private onPositionChangedCallback:
    | OnPositionChangedEventCallback
    | undefined = undefined;

  constructor(context: TContext, node: NNode) {
    super(context, node);

    this.detune = new AudioParam<TContext, NativeAudioContext>(
      node.detune,
      context
    );
    this.playbackRate = new AudioParam<TContext, NativeAudioContext>(
      node.playbackRate,
      context
    );
  }

  public get onPositionChanged(): OnPositionChangedEventCallback | undefined {
    return this.onPositionChangedCallback;
  }

  public set onPositionChanged(
    callback: OnPositionChangedEventCallback | null
  ) {
    if (!callback) {
      this.node.onPositionChanged = '0';
      this.positionChangedSubscription?.remove();
      this.positionChangedSubscription = undefined;
      this.onPositionChangedCallback = undefined;

      return;
    }

    this.onPositionChangedCallback = callback;
    this.positionChangedSubscription =
      this.audioEventEmitter.addAudioEventListener('positionChanged', callback);

    this.node.onPositionChanged =
      this.positionChangedSubscription.subscriptionId;
  }

  public get onPositionChangedInterval(): number {
    return this.node.onPositionChangedInterval;
  }

  public set onPositionChangedInterval(value: number) {
    this.node.onPositionChangedInterval = value;
  }
}
