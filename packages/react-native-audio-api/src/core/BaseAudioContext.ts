interface IRecorderAdapterNode {}
interface IOscillatorNode {}

export interface IBaseAudioContext {}

interface INativeAudioContext extends IBaseAudioContext {
  createMyMomNode(): IRecorderAdapterNode;
}

interface WebAudioContext
  extends IBaseAudioContext,
    globalThis.BaseAudioContext {}

class OscillatorNode<NativeContext extends IBaseAudioContext> {
  constructor(
    context: BaseAudioContext<NativeContext>,
    nativeNode: IOscillatorNode
  ) {
    (context.nativeContext as WebAudioContext).createDynamicsCompressor();
  }
}

class RecorderAdapterNode {
  constructor(
    context: BaseAudioContext<IBaseAudioContext>,
    nativeNode: IRecorderAdapterNode
  ) {}
}

export default class BaseAudioContext<NativeContext> {
  readonly nativeContext: NativeContext;
  readonly sampleRate: number;

  constructor(nativeContext: NativeContext) {
    this.nativeContext = nativeContext;
    this.sampleRate = nativeContext.sampleRate;
  }

  createOscillator(): OscillatorNode<NativeContext> {
    const nativeNode = this.nativeContext.createOscillator();

    return new OscillatorNode(this, nativeNode);
  }

  createRecorderAdapter(): RecorderAdapterNode {}
}
