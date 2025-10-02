// TS don't complain
interface IUniversalAudioNode {}

// src/types/internal/IAudioSekuNode.ts - interface definitions

export interface IUniversalAudioSekuNode extends IUniversalAudioNode {
  readonly knotsLength: number;
}

export interface IRNNativeAudioSekuNode extends IUniversalAudioSekuNode {
  readonly knots: Array<number>;
  setKnots(knots: Array<number>): void;
}

export interface IWebNativeAudioSekuNode extends IUniversalAudioSekuNode {
  readonly knots: Uint8Array;
  setKnots(knots: Uint8Array): void;
}

export type INativeAudioSekuNode =
  | IRNNativeAudioSekuNode
  | IWebNativeAudioSekuNode;

export interface IRNAudioSekuNode extends IUniversalAudioSekuNode {
  readonly knots: Array<number>;
  setKnots(knots: Array<number>): void;
}

// src/core/effects/BaseAudioSekuNode.ts - base class definition

export class BaseAudioSekuNode implements IUniversalAudioSekuNode {
  protected readonly node: INativeAudioSekuNode;

  constructor(node: INativeAudioSekuNode) {
    this.node = node;
  }

  public get knotsLength(): number {
    return this.node.knotsLength;
  }
}

// src/core/effects/AudioSekuNode.ts - native (RN default - but this doesn't matter) class definition

// NOTE: RN prefix is for single-file example/use-case
export class RNAudioSekuNode
  extends BaseAudioSekuNode
  implements IRNAudioSekuNode
{
  public get knots(): Array<number> {
    return (this.node as IRNNativeAudioSekuNode).knots;
  }

  public setKnots(knots: Array<number>): void {
    (this.node as IRNNativeAudioSekuNode).setKnots(knots);
  }
}

// src/core/effects/AudioSekuNode.web.ts - web class definition

// NOTE: Web prefix is for single-file example/use-case
export class WebAudioSekuNode
  extends BaseAudioSekuNode
  implements IRNAudioSekuNode
{
  public get knots(): Array<number> {
    return Array.from((this.node as IWebNativeAudioSekuNode).knots);
  }

  public setKnots(knots: Array<number>): void {
    (this.node as IWebNativeAudioSekuNode).setKnots(new Uint8Array(knots));
  }
}

interface BaseNode {
  readonly blabla: string;
}

interface Extension {
  readonly extension: string;
}

export class Example implements BaseNode, Extension {
  public get blabla(): string {
    return 'blabla';
  }

  public get extension(): string {
    return 'extension';
  }
}
