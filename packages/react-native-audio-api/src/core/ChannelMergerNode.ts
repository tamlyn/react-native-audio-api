import { IChannelMergerNode } from '../interfaces';
import AudioNode from './AudioNode';
import BaseAudioContext from './BaseAudioContext';

export default class ChannelMergerNode extends AudioNode {
  readonly numberOfInputs: number;

  constructor(context: BaseAudioContext, merger: IChannelMergerNode) {
    super(context, merger);
    this.numberOfInputs = merger.numberOfInputs;
  }
}
