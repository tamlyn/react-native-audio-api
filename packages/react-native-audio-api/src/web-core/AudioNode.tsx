import BaseAudioContext from './BaseAudioContext';
import { ChannelCountMode, ChannelInterpretation } from '../types';
import AudioParam from './AudioParam';

export default class AudioNode {
  readonly context: BaseAudioContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;

  protected readonly node: globalThis.AudioNode;

  constructor(context: BaseAudioContext, node: globalThis.AudioNode) {
    this.context = context;
    this.node = node;
    this.numberOfInputs = this.node.numberOfInputs;
    this.numberOfOutputs = this.node.numberOfOutputs;
    this.channelCount = this.node.channelCount;
    this.channelCountMode = this.node.channelCountMode;
    this.channelInterpretation = this.node.channelInterpretation;
  }

  public connect(
    destination: AudioNode | AudioParam,
    output = 0,
    input = 0
  ): AudioNode | void {
    if (destination instanceof AudioParam) {
      this.node.connect(destination.param, output);
      return;
    }

    if (destination instanceof AudioNode) {
      this.node.connect(destination.node, output, input);
      return destination;
    }
  }

  public disconnect(
    destinationOrOutput?: AudioNode | AudioParam | number,
    output?: number,
    input?: number
  ): void {
    if (destinationOrOutput === undefined) {
      this.node.disconnect();
      return;
    }

    if (typeof destinationOrOutput === 'number') {
      const outputIndex = destinationOrOutput;
      this.node.disconnect(outputIndex);
      return;
    }

    const destination = destinationOrOutput;

    if (destination instanceof AudioParam) {
      if (output !== undefined) {
        this.node.disconnect(destination.param, output);
      } else {
        this.node.disconnect(destination.param);
      }
      return;
    }

    if (destination instanceof AudioNode) {
      if (output !== undefined && input !== undefined) {
        this.node.disconnect(destination.node, output, input);
      } else if (output !== undefined) {
        this.node.disconnect(destination.node, output);
      } else {
        this.node.disconnect(destination.node);
      }
    }
  }
}
