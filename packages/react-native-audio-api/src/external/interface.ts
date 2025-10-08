export default interface IWasmLoader {
  load(pathPrefix: string): Promise<void>;
  getPromise(): Promise<void>;
}
