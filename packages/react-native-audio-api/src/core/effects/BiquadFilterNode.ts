import { InvalidAccessError } from '../../errors';
import { BiquadFilterType } from '../../types';
import type {
  IGenericBaseAudioContext,
  IGenericBiquadFilterNode,
} from '../../types/generics';
import AudioNode from '../AudioNode';
import AudioParam from '../AudioParam';

export default class BiquadFilterNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
  >
  extends AudioNode<TContext, NContext, IGenericBiquadFilterNode<NContext>>
  implements IGenericBiquadFilterNode<TContext>
{
  readonly frequency: AudioParam<TContext, NContext>;
  readonly detune: AudioParam<TContext, NContext>;
  readonly Q: AudioParam<TContext, NContext>;
  readonly gain: AudioParam<TContext, NContext>;

  constructor(
    context: TContext,
    biquadFilter: IGenericBiquadFilterNode<NContext>
  ) {
    super(context, biquadFilter);

    this.Q = new AudioParam<TContext, NContext>(biquadFilter.Q, context);
    this.gain = new AudioParam<TContext, NContext>(biquadFilter.gain, context);
    this.frequency = new AudioParam<TContext, NContext>(
      biquadFilter.frequency,
      context
    );
    this.detune = new AudioParam<TContext, NContext>(
      biquadFilter.detune,
      context
    );
  }

  public get type(): BiquadFilterType {
    return this.node.type;
  }

  public set type(value: BiquadFilterType) {
    this.node.type = value;
  }

  public getFrequencyResponse(
    frequencyArray: Float32Array,
    magResponseOutput: Float32Array,
    phaseResponseOutput: Float32Array
  ) {
    if (
      frequencyArray.length !== magResponseOutput.length ||
      frequencyArray.length !== phaseResponseOutput.length
    ) {
      throw new InvalidAccessError(
        `The lengths of the arrays are not the same frequencyArray: ${frequencyArray.length}, magResponseOutput: ${magResponseOutput.length}, phaseResponseOutput: ${phaseResponseOutput.length}`
      );
    }

    this.node.getFrequencyResponse(
      frequencyArray,
      magResponseOutput,
      phaseResponseOutput
    );
  }
}
