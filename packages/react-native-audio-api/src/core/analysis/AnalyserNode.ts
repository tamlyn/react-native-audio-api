import type { WindowType } from '../../types';
import type { IGenericBaseAudioContext } from '../../types/generics';
import BaseAnalyserNode, {
  IAbstractNativeAnalyserNode,
} from './BaseAnalyserNode';

// TODO: fixme - temporary any to avoid work
interface NativeAudioContext {}

interface MobileAnalyserNode
  extends IAbstractNativeAnalyserNode<NativeAudioContext> {
  window: WindowType;
}

export default class AnalyserNodeNative<
  TContext extends IGenericBaseAudioContext,
> extends BaseAnalyserNode<TContext, NativeAudioContext, MobileAnalyserNode> {
  public get window(): WindowType {
    return this.node.window;
  }

  public set window(value: WindowType) {
    this.node.window = value;
  }
}
