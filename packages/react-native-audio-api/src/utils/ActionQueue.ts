type AsyncAction<T = unknown> = () => T | Promise<T>;

export default class ActionQueue {
  private queue: AsyncAction[] = [];
  private isProcessing = false;

  /**
   * Add an async action to the queue.
   * @param action - A function that returns a Promise.
   * @returns A Promise that resolves or rejects with the action's result.
   */
  public enqueue<T>(action: AsyncAction<T>): Promise<T> {
    return new Promise<T>((resolve, reject) => {
      const wrappedAction = async () => {
        try {
          const result = await action();
          resolve(result);
        } catch (err) {
          reject(err);
        }
      };

      this.queue.push(wrappedAction);
      this.processQueue();
    });
  }

  /**
   * Internal method to process the queue.
   */
  private async processQueue(): Promise<void> {
    if (this.isProcessing || this.queue.length === 0) {
      return;
    }

    this.isProcessing = true;

    const nextAction = this.queue.shift();

    if (nextAction) {
      try {
        await nextAction();
      } finally {
        this.isProcessing = false;
        this.processQueue();
      }
    }
  }
}
