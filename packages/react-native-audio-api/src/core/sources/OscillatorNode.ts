import { InvalidStateError } from '../../errors';
import type { OscillatorType } from '../../types';
import type { IBaseAudioContext, IOscillatorNode } from '../../types/internal';
import AudioParam from '../AudioParam';
import PeriodicWave from '../PeriodicWave';

import AudioScheduledSourceNode from './AudioScheduledSourceNode';

export default class OscillatorNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioScheduledSourceNode<
    TContext,
    NContext,
    IOscillatorNode<NContext>
  >
  implements IOscillatorNode<TContext>
{
  readonly frequency: AudioParam<TContext, NContext>;
  readonly detune: AudioParam<TContext, NContext>;

  constructor(context: TContext, node: IOscillatorNode<NContext>) {
    super(context, node);

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

  public setPeriodicWave(wave: PeriodicWave): void {
    this.node.setPeriodicWave(wave.periodicWave);
  }
}
