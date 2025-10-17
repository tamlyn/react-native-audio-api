import { NativeEventEmitter, NativeModules } from 'react-native';
import NativeMedi from './NativeMedi';

// Get the event emitter module
const MediEventEmitterModule = NativeModules.MediEventEmitter;
const eventEmitter = MediEventEmitterModule
  ? new NativeEventEmitter(MediEventEmitterModule)
  : null;

// Event names matching Web MIDI API
export const MIDI_MESSAGE_EVENT = 'onMIDIMessage';
export const STATE_CHANGE_EVENT = 'onStateChange';

export interface NativeMIDIMessageEvent {
  portId: string;
  data: number[];
  timestamp: number;
}

export interface NativeStateChangeEvent {
  portId: string;
  state: 'connected' | 'disconnected';
  connection: 'open' | 'closed' | 'pending';
}

export interface MIDIMessageEvent {
  type: string;
  data: Uint8Array;
  timestamp: number;
}

type Subscription = { remove: () => void };

/** Event manager for MIDI events following Web MIDI API specifications */
export class MIDIEventManager {
  private messageListeners: Map<
    string,
    Set<(event: MIDIMessageEvent) => void>
  > = new Map();

  private messageSubscription: Subscription | null = null;
  private stateSubscription: Subscription | null = null;

  constructor() {
    if (eventEmitter) {
      this.setupNativeListeners();
    } else {
      console.warn('MediEventEmitter module not found');
    }
  }

  private setupNativeListeners() {
    if (!eventEmitter) return;

    // Listen for MIDI messages from native layer
    this.messageSubscription = eventEmitter.addListener(
      MIDI_MESSAGE_EVENT,
      (nativeEvent) => {
        const evt = nativeEvent as NativeMIDIMessageEvent;
        const listeners = this.messageListeners.get(evt.portId);
        if (listeners) {
          // Convert to Web MIDI API compatible event
          const event: MIDIMessageEvent = {
            type: 'midimessage',
            data: new Uint8Array(evt.data),
            timestamp: evt.timestamp,
          };

          listeners.forEach((listener) => listener(event));
        }
      }
    );

    // Listen for state changes from native layer
    this.stateSubscription = eventEmitter.addListener(
      STATE_CHANGE_EVENT,
      (nativeEvent) => {
        const evt = nativeEvent as NativeStateChangeEvent;
        console.log('State change:', evt);
      }
    );
  }

  /** Add listener for MIDI messages on a specific port (Web MIDI 'midimessage') */
  addMIDIMessageListener(
    portId: string,
    listener: (event: MIDIMessageEvent) => void
  ): void {
    if (!this.messageListeners.has(portId)) {
      this.messageListeners.set(portId, new Set());
      // Notify native layer that we have a listener for this port
      NativeMedi.addListener(portId);
    }
    this.messageListeners.get(portId)!.add(listener);
  }

  /** Remove listener for MIDI messages on a specific port */
  removeMIDIMessageListener(
    portId: string,
    listener: (event: MIDIMessageEvent) => void
  ): void {
    const listeners = this.messageListeners.get(portId);
    if (listeners) {
      listeners.delete(listener);
      if (listeners.size === 0) {
        this.messageListeners.delete(portId);
        // Notify native layer that we no longer have listeners for this port
        NativeMedi.removeListener(portId);
      }
    }
  }

  /** Clean up all listeners */
  removeAllListeners(): void {
    // Notify native layer for each port we were listening to
    this.messageListeners.forEach((_, portId) => {
      NativeMedi.removeListener(portId);
    });

    this.messageListeners.clear();

    if (this.messageSubscription) {
      this.messageSubscription.remove();
    }
    if (this.stateSubscription) {
      this.stateSubscription.remove();
    }
  }
}

// Export singleton instance
export const midiEventManager = new MIDIEventManager();
