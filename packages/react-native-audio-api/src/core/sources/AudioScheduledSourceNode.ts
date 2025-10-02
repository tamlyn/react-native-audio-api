import { AudioEventEmitter, AudioEventSubscription } from '../../events';
import type {
  IAudioScheduledSourceNode,
  IBaseAudioContext,
} from '../../types/internal';
import type { OnEndedEventCallback } from './AudioScheduledSourceNode.web';
import AudioScheduledSourceNode from './AudioScheduledSourceNode.web';

interface INativeAudioScheduledSourceNode<TContext extends IBaseAudioContext>
  extends IAudioScheduledSourceNode<TContext> {
  onEnded: string; // subscriptionId or '0' for none
}

export default class AudioScheduledSourceNodeNative<
  TContext extends IBaseAudioContext,
  NContext extends IBaseAudioContext,
  TNode extends
    INativeAudioScheduledSourceNode<NContext> = INativeAudioScheduledSourceNode<NContext>,
> extends AudioScheduledSourceNode<TContext, NContext, TNode> {
  protected readonly audioEventEmitter = new AudioEventEmitter(
    global.AudioEventEmitter
  );

  private onEndedSubscription?: AudioEventSubscription;
  private onEndedCallbackNative: OnEndedEventCallback | null = null;

  public get onEnded(): OnEndedEventCallback | undefined {
    return this.onEndedCallbackNative ?? undefined;
  }

  public set onEnded(callback: OnEndedEventCallback | null) {
    if (!callback) {
      this.node.onEnded = '0';
      this.onEndedSubscription?.remove();
      this.onEndedSubscription = undefined;
      this.onEndedCallbackNative = null;
      return;
    }

    this.onEndedCallbackNative = callback;
    this.onEndedSubscription = this.audioEventEmitter.addAudioEventListener(
      'ended',
      callback
    );

    this.node.onEnded = this.onEndedSubscription.subscriptionId;
  }
}
