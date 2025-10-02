import { IndexSizeError } from '../../errors';
import type { WindowType } from '../../types';
import type { IAnalyserNode, IBaseAudioContext } from '../../types/internal';
import { availabilityWarn } from '../../utils';
import AudioNode from '../AudioNode';

const allowedFFTSize = [
  32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
];

export default class AnalyserNode<
    TContext extends IBaseAudioContext,
    NContext extends IBaseAudioContext,
  >
  extends AudioNode<TContext, NContext, IAnalyserNode<NContext>>
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

  public get window(): WindowType {
    return 'blackman';
  }

  public set window(_value: WindowType) {
    availabilityWarn('window', 'web', '/');
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
}
