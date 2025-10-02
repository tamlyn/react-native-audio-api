import type { IGenericBaseAudioContext } from '../../types/generics';
import { OnEndedEventCallback } from '../../types/interfaces';
import BaseAudioScheduledSourceNode from './BaseAudioScheduledSourceNode';

type NativeAudioContext = globalThis.BaseAudioContext;
type NativeAudioScheduledSourceNode = globalThis.AudioScheduledSourceNode;

export default class AudioScheduledSourceNode<
  TContext extends IGenericBaseAudioContext,
> extends BaseAudioScheduledSourceNode<
  TContext,
  NativeAudioContext,
  NativeAudioScheduledSourceNode
> {
  private onEndedCallback: OnEndedEventCallback | undefined = undefined;

  public get onEnded(): OnEndedEventCallback | undefined {
    return this.onEndedCallback;
  }

  public set onEnded(callback: OnEndedEventCallback | undefined) {
    this.onEndedCallback = callback;

    if (!callback) {
      this.node.onended = null;
      return;
    }

    this.node.onended = () => {
      this.onEndedCallback?.({ bufferId: undefined });
    };
  }
}
