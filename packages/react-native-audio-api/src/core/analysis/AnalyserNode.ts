import type { WindowType } from '../../types';
import type { IAnalyserNode, IBaseAudioContext } from '../../types/internal';
import AnalyserNode from './AnalyserNode.web';

export default class AnalyserNodeNative<
  TContext extends IBaseAudioContext,
  NContext extends IBaseAudioContext,
> extends AnalyserNode<TContext, NContext> {
  public get window(): WindowType {
    return (this.node as IAnalyserNode<NContext>).window;
  }

  public set window(value: WindowType) {
    (this.node as IAnalyserNode<NContext>).window = value;
  }
}
