import { AudioEventEmitter, AudioEventSubscription } from '../../events';
import type { IGenericBaseAudioContext } from '../../types/generics';
import { OnEndedEventCallback } from '../../types/interfaces';
import BaseAudioScheduledSourceNode, {
  IAbstractNativeAudioScheduledSourceNode,
} from './BaseAudioScheduledSourceNode';

// TODO: fixme - temporary any to avoid work
interface NativeAudioContext {}

interface MobileAudioScheduledSourceNode
  extends IAbstractNativeAudioScheduledSourceNode<NativeAudioContext> {
  onEnded: string; // subscriptionId or '0' for none
}

export default class AudioScheduledSourceNodeNative<
  TContext extends IGenericBaseAudioContext,
  NNode extends MobileAudioScheduledSourceNode = MobileAudioScheduledSourceNode,
> extends BaseAudioScheduledSourceNode<TContext, NativeAudioContext, NNode> {
  protected readonly audioEventEmitter = new AudioEventEmitter(
    global.AudioEventEmitter
  );

  private onEndedSubscription?: AudioEventSubscription;
  private onEndedCallback: OnEndedEventCallback | null = null;

  public get onEnded(): OnEndedEventCallback | undefined {
    return this.onEndedCallback ?? undefined;
  }

  public set onEnded(callback: OnEndedEventCallback | null) {
    if (!callback) {
      this.node.onEnded = '0';
      this.onEndedSubscription?.remove();
      this.onEndedSubscription = undefined;
      this.onEndedCallback = null;
      return;
    }

    this.onEndedCallback = callback;
    this.onEndedSubscription = this.audioEventEmitter.addAudioEventListener(
      'ended',
      callback
    );

    this.node.onEnded = this.onEndedSubscription.subscriptionId;
  }
}
