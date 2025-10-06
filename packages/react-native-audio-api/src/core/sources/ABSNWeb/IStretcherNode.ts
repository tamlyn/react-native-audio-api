export interface ScheduleOptions {
  rate?: number;
  active?: boolean;
  output?: number;
  input?: number;
  semitones?: number;
  loopStart?: number;
  loopEnd?: number;
}

export default interface IStretcherNode extends globalThis.AudioNode {
  channelCount: number;
  channelCountMode: globalThis.ChannelCountMode;
  channelInterpretation: globalThis.ChannelInterpretation;
  context: globalThis.BaseAudioContext;
  numberOfInputs: number;
  numberOfOutputs: number;

  onended:
    | ((this: globalThis.AudioScheduledSourceNode, ev: Event) => unknown)
    | null;
  addEventListener: (
    type: string,
    listener: EventListenerOrEventListenerObject | null,
    options?: boolean | AddEventListenerOptions | undefined
  ) => void;
  dispatchEvent: (event: Event) => boolean;
  removeEventListener: (
    type: string,
    callback: EventListenerOrEventListenerObject | null,
    options?: boolean | EventListenerOptions | undefined
  ) => void;

  addBuffers(channels: Float32Array[]): void;
  dropBuffers(): void;

  schedule(options: ScheduleOptions): void;

  start(
    when?: number,
    offset?: number,
    duration?: number,
    rate?: number,
    semitones?: number
  ): void;

  stop(when?: number): void;

  connect(
    destination: globalThis.AudioNode,
    output?: number,
    input?: number
  ): globalThis.AudioNode;
  connect(destination: globalThis.AudioParam, output?: number): void;

  disconnect(): void;
  disconnect(output: number): void;

  disconnect(destination: globalThis.AudioNode): globalThis.AudioNode;
  disconnect(destination: globalThis.AudioNode, output: number): void;
  disconnect(
    destination: globalThis.AudioNode,
    output: number,
    input: number
  ): void;

  disconnect(destination: globalThis.AudioParam): void;
  disconnect(destination: globalThis.AudioParam, output: number): void;
}
