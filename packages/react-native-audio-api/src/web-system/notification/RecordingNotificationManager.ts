/* eslint-disable @typescript-eslint/no-unused-vars */
/* eslint-disable no-useless-constructor */
/* eslint-disable @typescript-eslint/require-await */

import type { AudioEventSubscription } from '../../events';
import type {
  NotificationEvents,
  NotificationManager,
  RecordingControlName,
  RecordingNotificationEventName,
  RecordingNotificationInfo,
} from '../../system';

/// Mock Manager for recording notifications. Does nothing.
class RecordingNotificationManager
  implements
    NotificationManager<
      RecordingNotificationInfo,
      RecordingNotificationInfo,
      RecordingNotificationEventName
    >
{
  private isRegistered_ = false;
  private isShown_ = false;

  constructor() {}

  async register(): Promise<void> {}

  async show(info: RecordingNotificationInfo): Promise<void> {}

  async update(info: RecordingNotificationInfo): Promise<void> {}

  async hide(): Promise<void> {}

  async unregister(): Promise<void> {}

  async enableControl(
    control: RecordingControlName,
    enabled: boolean
  ): Promise<void> {}

  async isActive(): Promise<boolean> {
    return this.isShown_;
  }

  isRegistered(): boolean {
    return this.isRegistered_;
  }

  addEventListener<T extends RecordingNotificationEventName>(
    eventName: T,
    callback: (event: NotificationEvents[T]) => void
  ): AudioEventSubscription {
    // dummy subscription object with a no-op remove method
    return {
      remove: () => {},
    } as unknown as AudioEventSubscription;
  }

  removeEventListener(subscription: AudioEventSubscription): void {}
}

export default new RecordingNotificationManager();
