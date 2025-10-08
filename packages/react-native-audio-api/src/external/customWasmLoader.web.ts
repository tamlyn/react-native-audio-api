import { eventTitle, globalTag } from './constants';
import type IWasmLoader from './interface';

class CustomWasmLoader implements IWasmLoader {
  private loadPromise: Promise<void> | null = null;

  async load(pathPrefix: string = ''): Promise<void> {
    if (typeof window === 'undefined' || typeof document === 'undefined') {
      return Promise.resolve();
    }

    if (this.loadPromise) {
      return this.loadPromise;
    }

    this.loadPromise = new Promise<void>((resolve, reject) => {
      const scriptTag = document.createElement('script');
      scriptTag.type = 'module';

      scriptTag.textContent = `
        import SignalsmithStretch from '${pathPrefix}/signalsmithStretch.mjs';
        window.${globalTag} = SignalsmithStretch;
        window.postMessage('${eventTitle}');
      `;

      function onLoaded(event: MessageEvent<string>) {
        if (event.data !== eventTitle) {
          reject(new Error(`Unexpected event received: ${event.data}`));
          return;
        }

        resolve();
        window.removeEventListener('message', onLoaded);
      }

      window.addEventListener('message', onLoaded);
      document.head.appendChild(scriptTag);
    });

    return this.loadPromise;
  }

  getPromise(): Promise<void> {
    if (!this.loadPromise) {
      return Promise.reject(
        new Error('WASM not loaded yet. Call load() first.')
      );
    }

    return this.loadPromise;
  }
}

export default new CustomWasmLoader();
