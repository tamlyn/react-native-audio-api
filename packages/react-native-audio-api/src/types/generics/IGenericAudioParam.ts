export default interface IGenericAudioParam {
  readonly defaultValue: number;
  readonly minValue: number;
  readonly maxValue: number;

  value: number;

  cancelAndHoldAtTime: (cancelTime: number) => IGenericAudioParam;
  cancelScheduledValues: (cancelTime: number) => IGenericAudioParam;
  exponentialRampToValueAtTime: (
    value: number,
    endTime: number
  ) => IGenericAudioParam;
  linearRampToValueAtTime: (
    value: number,
    endTime: number
  ) => IGenericAudioParam;
  setTargetAtTime: (
    target: number,
    startTime: number,
    timeConstant: number
  ) => IGenericAudioParam;
  setValueAtTime: (value: number, startTime: number) => IGenericAudioParam;
  setValueCurveAtTime: (
    values: Float32Array,
    startTime: number,
    duration: number
  ) => IGenericAudioParam;
}
