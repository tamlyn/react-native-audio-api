import { IConvolverNode } from '../interfaces';
import { ConvolverOptions } from '../defaults';
import { TConvolverOptions } from '../types';
import { NotSupportedError } from '../errors';
import BaseAudioContext from './BaseAudioContext';
import AudioNode from './AudioNode';
import AudioBuffer from './AudioBuffer';

export default class ConvolverNode extends AudioNode {
  constructor(context: BaseAudioContext, options?: TConvolverOptions) {
    const finalOptions: TConvolverOptions = {
      ...ConvolverOptions,
      ...options,
    };
    if (finalOptions.buffer) {
      const numberOfChannels = finalOptions.buffer.numberOfChannels;
      if (
        numberOfChannels !== 1 &&
        numberOfChannels !== 2 &&
        numberOfChannels !== 4
      ) {
        throw new NotSupportedError(
          `The number of channels provided (${numberOfChannels}) in impulse response for ConvolverNode buffer must be 1 or 2 or 4.`
        );
      }
    }
    const convolverNode: IConvolverNode =
      context.context.createConvolver(finalOptions);
    super(context, convolverNode);
    this.normalize = convolverNode.normalize;
  }

  public get buffer(): AudioBuffer | null {
    const buffer = (this.node as IConvolverNode).buffer;
    if (!buffer) {
      return null;
    }
    return new AudioBuffer(buffer);
  }

  public set buffer(buffer: AudioBuffer | null) {
    if (!buffer) {
      (this.node as IConvolverNode).buffer = null;
      return;
    }
    (this.node as IConvolverNode).buffer = buffer.buffer;
  }

  public get normalize(): boolean {
    return (this.node as IConvolverNode).normalize;
  }

  public set normalize(value: boolean) {
    (this.node as IConvolverNode).normalize = value;
  }
}
