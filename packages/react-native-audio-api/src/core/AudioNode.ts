import { IAudioNode } from '../interfaces';
import AudioParam from './AudioParam';
import { ChannelCountMode, ChannelInterpretation } from '../types';
import BaseAudioContext from './BaseAudioContext';
import {
  InvalidAccessError,
  IndexSizeError,
  NotSupportedError,
} from '../errors';

export default class AudioNode {
  readonly context: BaseAudioContext;
  readonly numberOfInputs: number;
  readonly numberOfOutputs: number;
  readonly channelCount: number;
  readonly channelCountMode: ChannelCountMode;
  readonly channelInterpretation: ChannelInterpretation;
  protected readonly node: IAudioNode;

  constructor(context: BaseAudioContext, node: IAudioNode) {
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
    outputIndex: number = 0,
    inputIndex: number = 0
  ): AudioNode | AudioParam | undefined {
    if (this.context !== destination.context) {
      throw new InvalidAccessError(
        'Source and destination are from different BaseAudioContexts'
      );
    }
    if (outputIndex < 0 || outputIndex >= this.numberOfOutputs) {
      throw new IndexSizeError('Output index is out of range');
    }
    if (destination instanceof AudioNode) {
      if (inputIndex < 0 || inputIndex >= destination.numberOfInputs) {
        throw new IndexSizeError('Input index is out of range');
      }
    }
    if (destination instanceof AudioParam && inputIndex !== 0) {
      throw new NotSupportedError(
        'Cannot specify input index when connecting to an AudioParam'
      );
    }

    if (destination instanceof AudioParam) {
      this.node.connect(destination.audioParam, outputIndex);
      return undefined;
    } else {
      this.node.connect(destination.node, outputIndex, inputIndex);
    }

    return destination;
  }

  public disconnect(
    destinationOrOutput?: AudioNode | AudioParam | number,
    output?: number,
    input?: number
  ): void {
    // probably needs a switch statement
    // Case 1: disconnect()
    if (destinationOrOutput === undefined) {
      this.node.disconnect();
      return;
    }

    // Case 2: disconnect(outputIndex: number)
    if (typeof destinationOrOutput === 'number') {
      this.node.disconnect(destinationOrOutput);
      return;
    }

    const destination = destinationOrOutput;

    if (destination instanceof AudioParam) {
      // Destination is an AudioParam
      if (output === undefined) {
        // disconnect(destination: AudioParam)
        this.node.disconnect(destination.audioParam);
      } else {
        // disconnect(destination: AudioParam, output: number)
        this.node.disconnect(destination.audioParam, output);
      }
    } else {
      // Destination is an AudioNode
      if (output === undefined) {
        // disconnect(destination: AudioNode)
        this.node.disconnect(destination.node);
      } else if (input === undefined) {
        // disconnect(destination: AudioNode, output: number)
        this.node.disconnect(destination.node, output);
      } else {
        // disconnect(destination: AudioNode, output: number, input: number)
        this.node.disconnect(destination.node, output, input);
      }
    }
  }
}
