import { InvalidStateError } from '../errors';
import {
  IGenericAudioParam,
  IGenericBaseAudioContext,
} from '../types/generics';

export default class AudioParam<
  TContext extends IGenericBaseAudioContext,
  NContext extends IGenericBaseAudioContext,
> implements IGenericAudioParam<TContext>
{
  readonly defaultValue: number;
  readonly minValue: number;
  readonly maxValue: number;
  readonly param: IGenericAudioParam<NContext>;
  readonly context: TContext;

  constructor(param: IGenericAudioParam<NContext>, context: TContext) {
    this.param = param;
    this.context = context;
    this.defaultValue = param.defaultValue;
    this.minValue = param.minValue;
    this.maxValue = param.maxValue;
  }

  public get value(): number {
    return this.param.value;
  }

  public set value(value: number) {
    this.param.value = value;
  }

  public setValueAtTime(
    value: number,
    startTime: number
  ): AudioParam<TContext, NContext> {
    if (startTime < 0) {
      throw new RangeError(
        `startTime must be a finite non-negative number: ${startTime}`
      );
    }

    this.param.setValueAtTime(value, startTime);

    return this;
  }

  public linearRampToValueAtTime(
    value: number,
    endTime: number
  ): AudioParam<TContext, NContext> {
    if (endTime < 0) {
      throw new RangeError(
        `endTime must be a finite non-negative number: ${endTime}`
      );
    }

    this.param.linearRampToValueAtTime(value, endTime);

    return this;
  }

  public exponentialRampToValueAtTime(
    value: number,
    endTime: number
  ): AudioParam<TContext, NContext> {
    if (endTime < 0) {
      throw new RangeError(
        `endTime must be a finite non-negative number: ${endTime}`
      );
    }

    this.param.exponentialRampToValueAtTime(value, endTime);

    return this;
  }

  public setTargetAtTime(
    target: number,
    startTime: number,
    timeConstant: number
  ): AudioParam<TContext, NContext> {
    if (startTime < 0) {
      throw new RangeError(
        `startTime must be a finite non-negative number: ${startTime}`
      );
    }

    if (timeConstant < 0) {
      throw new RangeError(
        `timeConstant must be a finite non-negative number: ${startTime}`
      );
    }

    this.param.setTargetAtTime(target, startTime, timeConstant);

    return this;
  }

  public setValueCurveAtTime(
    values: Float32Array,
    startTime: number,
    duration: number
  ): AudioParam<TContext, NContext> {
    if (startTime < 0) {
      throw new RangeError(
        `startTime must be a finite non-negative number: ${startTime}`
      );
    }

    if (duration < 0) {
      throw new RangeError(
        `duration must be a finite non-negative number: ${startTime}`
      );
    }

    if (values.length < 2) {
      throw new InvalidStateError(`values must contain at least two values`);
    }

    this.param.setValueCurveAtTime(values, startTime, duration);

    return this;
  }

  public cancelScheduledValues(
    cancelTime: number
  ): AudioParam<TContext, NContext> {
    if (cancelTime < 0) {
      throw new RangeError(
        `cancelTime must be a finite non-negative number: ${cancelTime}`
      );
    }

    this.param.cancelScheduledValues(cancelTime);

    return this;
  }

  public cancelAndHoldAtTime(
    cancelTime: number
  ): AudioParam<TContext, NContext> {
    if (cancelTime < 0) {
      throw new RangeError(
        `cancelTime must be a finite non-negative number: ${cancelTime}`
      );
    }

    this.param.cancelAndHoldAtTime(cancelTime);

    return this;
  }
}
