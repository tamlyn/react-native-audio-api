import { InvalidAccessError } from '../errors';
import { BiquadFilterType } from '../types';
import type { IBaseAudioContext, IBiquadFilterNode } from '../types/internal';
import AudioNode from './AudioNode';
import AudioParam from './AudioParam';

export default class BiquadFilterNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioNode<TContext, NContext>
  implements IBiquadFilterNode<TContext>
{
  readonly frequency: AudioParam<TContext, NContext>;
  readonly detune: AudioParam<TContext, NContext>;
  readonly Q: AudioParam<TContext, NContext>;
  readonly gain: AudioParam<TContext, NContext>;

  constructor(context: TContext, biquadFilter: IBiquadFilterNode<NContext>) {
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
    return (this.node as IBiquadFilterNode<NContext>).type;
  }

  public set type(value: BiquadFilterType) {
    (this.node as IBiquadFilterNode<NContext>).type = value;
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

    (this.node as IBiquadFilterNode<NContext>).getFrequencyResponse(
      frequencyArray,
      magResponseOutput,
      phaseResponseOutput
    );
  }
}
