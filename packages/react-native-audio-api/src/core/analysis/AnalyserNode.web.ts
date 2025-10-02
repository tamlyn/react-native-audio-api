import type { WindowType } from '../../types';
import type { IGenericBaseAudioContext } from '../../types/generics';
import { availabilityWarn } from '../../utils';
import BaseAnalyserNode from './BaseAnalyserNode';

type NativeAnalyserNode = globalThis.AnalyserNode;
type NativeAudioContext = globalThis.BaseAudioContext;

export default class AnalyserNode<
  TContext extends IGenericBaseAudioContext,
> extends BaseAnalyserNode<TContext, NativeAudioContext, NativeAnalyserNode> {
  public get window(): WindowType {
    return 'blackman';
  }

  public set window(_value: WindowType) {
    availabilityWarn('window', 'web', '/');
  }
}
