import { InvalidStateError } from '../../errors';
import type { EventEmptyType } from '../../events';
import type { OscillatorType } from '../../types';
import type {
  IGenericBaseAudioContext,
  IGenericOscillatorNode,
} from '../../types/generics';
import AudioParam from '../AudioParam';
import PeriodicWave from '../PeriodicWave';

import AudioScheduledSourceNode from './AudioScheduledSourceNode';

export default class OscillatorNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
  >
  // Baka! check explanation in IGenericOscillatorNode
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  extends AudioScheduledSourceNode<TContext, any>
  implements IGenericOscillatorNode<TContext>
{
  readonly frequency: AudioParam<TContext, NContext>;
  readonly detune: AudioParam<TContext, NContext>;
  // redefinition due to typescript limitation with generics and inheritance
  protected readonly node: IGenericOscillatorNode<NContext>;

  constructor(context: TContext, node: IGenericOscillatorNode<NContext>) {
    super(context, node);
    this.node = node;

    this.frequency = new AudioParam<TContext, NContext>(
      node.frequency,
      context
    );

    this.detune = new AudioParam<TContext, NContext>(node.detune, context);
  }

  public get type(): OscillatorType {
    return this.node.type;
  }

  public set type(value: OscillatorType) {
    if (value === 'custom') {
      throw new InvalidStateError(
        "'type' cannot be set directly to 'custom'.  Use setPeriodicWave() to create a custom Oscillator type."
      );
    }

    this.node.type = value;
  }

  public setPeriodicWave(wave: PeriodicWave<TContext, NContext>): void {
    this.node.setPeriodicWave(wave.periodicWave);
  }

  public override get onEnded(): ((event: EventEmptyType) => void) | undefined {
    return super.onEnded as ((event: EventEmptyType) => void) | undefined;
  }

  public override set onEnded(
    callback: ((event: EventEmptyType) => void) | null
  ) {
    super.onEnded = callback;
  }
}
