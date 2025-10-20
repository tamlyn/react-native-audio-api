import type {
  IGenericBaseAudioContext,
  IGenericRecorderAdapterNode,
} from '../../types/generics';
import { availabilityWarn } from '../../utils';

export default class RecorderAdapterNode<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
> implements IGenericRecorderAdapterNode<TContext>
{
  public numberOfInputs: number = 1;
  public numberOfOutputs: number = 1;
  public channelCount: number = 2;
  public channelCountMode: 'max' | 'clamped-max' | 'explicit' = 'max';
  public channelInterpretation: 'speakers' | 'discrete' = 'speakers';

  /** @internal */
  public wasConnected: boolean = false;

  private node: IGenericRecorderAdapterNode<NContext>;
  public context: TContext;

  constructor(context: TContext, node: IGenericRecorderAdapterNode<NContext>) {
    this.node = node;
    this.context = context;
  }

  /** @internal */
  public getNode(): IGenericRecorderAdapterNode<NContext> {
    availabilityWarn('RecorderAdapterNode', 'web');
    // TODO: fishy
    return {} as IGenericRecorderAdapterNode<NContext>;
  }
}
