import { IChannelSplitterNode } from '../interfaces';
import AudioNode from './AudioNode';
import BaseAudioContext from './BaseAudioContext';

export default class ChannelSplitterNode extends AudioNode {
  readonly numberOfOutputs: number;

  constructor(context: BaseAudioContext, splitter: IChannelSplitterNode) {
    super(context, splitter);
    this.numberOfOutputs = splitter.numberOfOutputs;
  }
}
