import { customWasmLoader, globalTag } from '../../../external';
import IStretcherNode from './IStretcherNode';

type TContext = globalThis.AudioContext;

declare global {
  interface Window {
    [globalTag]?: (context: TContext) => Promise<IStretcherNode>;
  }
}

class StretcherProvider {
  private _stretcherNode: IStretcherNode | null = null;
  private _isAvailable: boolean | null = null;
  private _context: TContext;

  constructor(context: TContext) {
    this._context = context;

    this.createStretcherNode();
  }

  private async createStretcherNode(): Promise<void> {
    try {
      if (this._stretcherNode) {
        return;
      }

      await customWasmLoader.getPromise();

      if (typeof window === 'undefined' || !window[globalTag]) {
        return;
      }

      const node = await window[globalTag](this._context);

      this._stretcherNode = node;
      this._isAvailable = true;
    } catch {
      throw new Error('Failed to load stretcher node');
    }
  }

  getStretcherNode(): IStretcherNode {
    if (this._isAvailable === false || !this._stretcherNode) {
      throw new Error('Stretcher node is not available');
    }

    const node = this._stretcherNode;
    this._stretcherNode = null;
    this._isAvailable = null;

    this.createStretcherNode();

    return node;
  }
}

export default StretcherProvider;
