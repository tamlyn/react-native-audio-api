import type { WindowType } from '../../types';
import type { IBaseAudioContext } from '../../types/internal';
import AnalyserNode from './AnalyserNode.web';

export default class AnalyserNodeNative<
  TContext extends IBaseAudioContext,
  NContext extends IBaseAudioContext,
> extends AnalyserNode<TContext, NContext> {
  public get window(): WindowType {
    return this.node.window;
  }

  public set window(value: WindowType) {
    this.node.window = value;
  }
}
