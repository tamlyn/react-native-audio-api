export default interface IAudioParam {
  readonly defaultValue: number;
  readonly minValue: number;
  readonly maxValue: number;

  value: number;

  cancelAndHoldAtTime: (cancelTime: number) => IAudioParam;
  cancelScheduledValues: (cancelTime: number) => IAudioParam;
  exponentialRampToValueAtTime: (value: number, endTime: number) => IAudioParam;
  linearRampToValueAtTime: (value: number, endTime: number) => IAudioParam;
  setTargetAtTime: (
    target: number,
    startTime: number,
    timeConstant: number
  ) => IAudioParam;
  setValueAtTime: (value: number, startTime: number) => IAudioParam;
  setValueCurveAtTime: (
    values: Float32Array,
    startTime: number,
    duration: number
  ) => IAudioParam;
}
