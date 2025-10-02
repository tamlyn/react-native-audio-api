import { IndexSizeError } from '../../errors';
import type { WindowType } from '../../types';
import type {
  IGenericAudioNode,
  IGenericBaseAudioContext,
} from '../../types/generics';
import type { IAnalyserNode } from '../../types/interfaces';
import AudioNode from '../AudioNode';

// "Abstract" (TODO: better naming?) interface to describe the generalized native/web implementation
// it is used as generic for BaseAnalyserNode to allow for extensions
// TODO: should be exported?
export interface IAbstractNativeAnalyserNode<
  NContext extends IGenericBaseAudioContext,
> extends IGenericAudioNode<NContext> {
  fftSize: number;
  minDecibels: number;
  maxDecibels: number;
  smoothingTimeConstant: number;
  readonly frequencyBinCount: number;
  getFloatFrequencyData(array: Float32Array): void;
  getByteFrequencyData(array: Uint8Array): void;
  getFloatTimeDomainData(array: Float32Array): void;
  getByteTimeDomainData(array: Uint8Array): void;
}

export const allowedFFTSize = [
  32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
];

export default abstract class BaseAnalyserNode<
    TContext extends IGenericBaseAudioContext,
    NContext extends IGenericBaseAudioContext,
    NNode extends
      IAbstractNativeAnalyserNode<NContext> = IAbstractNativeAnalyserNode<NContext>,
  >
  extends AudioNode<TContext, NContext, NNode>
  implements IAnalyserNode<TContext>
{
  public get fftSize(): number {
    return this.node.fftSize;
  }

  public set fftSize(value: number) {
    if (!allowedFFTSize.includes(value)) {
      throw new IndexSizeError(
        `Provided value (${value}) must be a power of 2 between 32 and 32768`
      );
    }

    this.node.fftSize = value;
  }

  public get minDecibels(): number {
    return this.node.minDecibels;
  }

  public set minDecibels(value: number) {
    if (value >= this.node.maxDecibels) {
      throw new IndexSizeError(
        `The minDecibels value (${value}) must be less than maxDecibels`
      );
    }

    this.node.minDecibels = value;
  }

  public get smoothingTimeConstant(): number {
    return this.node.smoothingTimeConstant;
  }

  public set smoothingTimeConstant(value: number) {
    if (value < 0 || value > 1) {
      throw new IndexSizeError(
        `The smoothingTimeConstant value (${value}) must be between 0 and 1`
      );
    }

    this.node.smoothingTimeConstant = value;
  }

  public get maxDecibels(): number {
    return this.node.maxDecibels;
  }

  public set maxDecibels(value: number) {
    if (value <= this.node.minDecibels) {
      throw new IndexSizeError(
        `The maxDecibels value (${value}) must be greater than minDecibels`
      );
    }

    this.node.maxDecibels = value;
  }

  public get frequencyBinCount(): number {
    return this.node.frequencyBinCount;
  }

  public getFloatFrequencyData(array: Float32Array): void {
    this.node.getFloatFrequencyData(array);
  }

  public getByteFrequencyData(array: Uint8Array): void {
    this.node.getByteFrequencyData(array);
  }

  public getFloatTimeDomainData(array: Float32Array): void {
    this.node.getFloatTimeDomainData(array);
  }

  public getByteTimeDomainData(array: Uint8Array): void {
    this.node.getByteTimeDomainData(array);
  }

  public abstract get window(): WindowType;
  public abstract set window(value: WindowType);
}
