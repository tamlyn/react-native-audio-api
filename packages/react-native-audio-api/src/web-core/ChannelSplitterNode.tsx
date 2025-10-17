import AudioNode from './AudioNode';
import BaseAudioContext from './BaseAudioContext';

export default class ChannelSplitterNode extends AudioNode {
  readonly numberOfOutputs: number;

  constructor(
    context: BaseAudioContext,
    splitter: globalThis.ChannelSplitterNode
  ) {
    super(context, splitter);
    this.numberOfOutputs = splitter.numberOfOutputs;
  }
}
