import type IWasmLoader from './interface';

class CustomWasmLoader implements IWasmLoader {
  private loadPromise: Promise<void> = Promise.resolve();

  async load(_pathPrefix: string = ''): Promise<void> {
    // No-op on native
    return this.loadPromise;
  }

  getPromise(): Promise<void> {
    return this.loadPromise;
  }
}

export default new CustomWasmLoader();
