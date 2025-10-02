import IGenericBaseAudioContext from './IGenericBaseAudioContext';

export default interface IGenericAudioParam<
  TContext extends IGenericBaseAudioContext,
> {
  readonly defaultValue: number;
  readonly minValue: number;
  readonly maxValue: number;

  value: number;

  cancelAndHoldAtTime: (cancelTime: number) => IGenericAudioParam<TContext>;
  cancelScheduledValues: (cancelTime: number) => IGenericAudioParam<TContext>;
  exponentialRampToValueAtTime: (
    value: number,
    endTime: number
  ) => IGenericAudioParam<TContext>;
  linearRampToValueAtTime: (
    value: number,
    endTime: number
  ) => IGenericAudioParam<TContext>;
  setTargetAtTime: (
    target: number,
    startTime: number,
    timeConstant: number
  ) => IGenericAudioParam<TContext>;
  setValueAtTime: (
    value: number,
    startTime: number
  ) => IGenericAudioParam<TContext>;
  setValueCurveAtTime: (
    values: Float32Array,
    startTime: number,
    duration: number
  ) => IGenericAudioParam<TContext>;
}
