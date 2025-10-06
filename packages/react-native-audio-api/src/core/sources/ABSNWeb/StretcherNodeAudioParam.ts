export default class StretcherNodeAudioParam implements globalThis.AudioParam {
  private _value: number;
  private _setter: (value: number, when?: number) => void;

  public automationRate: AutomationRate;
  public defaultValue: number;
  public maxValue: number;
  public minValue: number;

  constructor(
    value: number,
    setter: (value: number, when?: number) => void,
    automationRate: AutomationRate,
    minValue: number,
    maxValue: number,
    defaultValue: number
  ) {
    this._value = value;
    this.automationRate = automationRate;
    this.minValue = minValue;
    this.maxValue = maxValue;
    this.defaultValue = defaultValue;
    this._setter = setter;
  }

  public get value(): number {
    return this._value;
  }

  public set value(value: number) {
    this._value = value;

    this._setter(value);
  }

  cancelAndHoldAtTime(cancelTime: number): globalThis.AudioParam {
    this._setter(this.defaultValue, cancelTime);
    return this;
  }

  cancelScheduledValues(cancelTime: number): globalThis.AudioParam {
    this._setter(this.defaultValue, cancelTime);
    return this;
  }

  exponentialRampToValueAtTime(
    _value: number,
    _endTime: number
  ): globalThis.AudioParam {
    console.warn(
      'exponentialRampToValueAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  linearRampToValueAtTime(
    _value: number,
    _endTime: number
  ): globalThis.AudioParam {
    console.warn(
      'linearRampToValueAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  setTargetAtTime(
    _target: number,
    _startTime: number,
    _timeConstant: number
  ): globalThis.AudioParam {
    console.warn(
      'setTargetAtTime is not implemented for pitch correction mode'
    );
    return this;
  }

  setValueAtTime(value: number, startTime: number): globalThis.AudioParam {
    this._setter(value, startTime);
    return this;
  }

  setValueCurveAtTime(
    _values: Float32Array,
    _startTime: number,
    _duration: number
  ): globalThis.AudioParam {
    console.warn(
      'setValueCurveAtTime is not implemented for pitch correction mode'
    );
    return this;
  }
}
