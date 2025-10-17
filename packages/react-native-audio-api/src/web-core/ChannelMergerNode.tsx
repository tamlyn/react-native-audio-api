import AudioNode from './AudioNode';
import BaseAudioContext from './BaseAudioContext';

export default class ChannelMergerNode extends AudioNode {
  readonly numberOfInputs: number;

  constructor(context: BaseAudioContext, merger: globalThis.ChannelMergerNode) {
    super(context, merger);
    this.numberOfInputs = merger.numberOfInputs;
  }
}
