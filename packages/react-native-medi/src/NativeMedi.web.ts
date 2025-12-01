import type {
  MIDIPortInfo,
  MIDIMessage,
  MIDIPortType,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
} from './types';

// Web MIDI API types
interface WebMIDIPort {
  id: string;
  manufacturer?: string;
  name?: string;
  type: 'input' | 'output';
  version?: string;
  state: 'connected' | 'disconnected';
  connection: 'open' | 'closed' | 'pending';
}

interface WebMIDIInput extends WebMIDIPort {
  type: 'input';
  onmidimessage?:
    | ((event: { data: Uint8Array; timeStamp: number }) => void)
    | null;
}

interface WebMIDIOutput extends WebMIDIPort {
  type: 'output';
  send(data: Uint8Array, timestamp?: number): void;
}

interface WebMIDIAccess {
  inputs: Map<string, WebMIDIInput>;
  outputs: Map<string, WebMIDIOutput>;
  sysexEnabled: boolean;
  onstatechange?: ((event: unknown) => void) | null;
}

// EventEmitter implementation for web
class EventEmitter {
  private listeners: ((data: MIDIMessage) => void)[] = [];

  addListener(listener: (data: MIDIMessage) => void): { remove: () => void } {
    this.listeners.push(listener);
    return {
      remove: () => {
        const index = this.listeners.indexOf(listener);
        if (index > -1) {
          this.listeners.splice(index, 1);
        }
      },
    };
  }

  emit(data: MIDIMessage): void {
    this.listeners.forEach((listener) => listener(data));
  }
}

class WebMediModule {
  private midiAccess: WebMIDIAccess | null = null;

  private openPorts = new Set<string>();

  private activeListeners = new Map<
    string,
    (event: { data: Uint8Array; timeStamp: number }) => void
  >();

  private eventEmitter = new EventEmitter();

  readonly onMidiMessage = this.eventEmitter.addListener.bind(
    this.eventEmitter
  );

  async prepareMIDIClient(sysex: boolean = false): Promise<void> {
    if (typeof window === 'undefined' || !window.navigator?.requestMIDIAccess) {
      throw new Error('Web MIDI API is not supported in this browser');
    }

    try {
      this.midiAccess = await window.navigator.requestMIDIAccess({
        sysex,
      });

      // Set up listeners for all input ports
      this.setupInputListeners();

      // Listen for port state changes
      if (this.midiAccess && this.midiAccess.onstatechange !== undefined) {
        this.midiAccess.onstatechange = () => {
          this.setupInputListeners();
        };
      }
    } catch (error) {
      const errorMessage =
        error instanceof Error ? error.message : 'Unknown error';
      throw new Error(`Failed to access MIDI: ${errorMessage}`);
    }
  }

  private setupInputListeners(): void {
    if (!this.midiAccess) return;

    this.midiAccess.inputs.forEach((input) => {
      if (input.onmidimessage !== undefined) {
        // Remove existing listener if any
        if (this.activeListeners.has(input.id)) {
          input.onmidimessage = null;
          this.activeListeners.delete(input.id);
        }

        // Only set up listener if port is open
        if (this.openPorts.has(input.id)) {
          const listener = (event: { data: Uint8Array; timeStamp: number }) => {
            this.eventEmitter.emit({
              portId: input.id,
              data: Array.from(event.data),
              timestamp: event.timeStamp || Date.now(),
            });
          };

          input.onmidimessage = listener;
          this.activeListeners.set(input.id, listener);
        }
      }
    });
  }

  getSources(): MIDIPortInfo[] {
    if (!this.midiAccess) {
      return [];
    }

    const sources: MIDIPortInfo[] = [];
    this.midiAccess.inputs.forEach((input) => {
      sources.push(this.webPortToMIDIPortInfo(input));
    });
    return sources;
  }

  getDestinations(): MIDIPortInfo[] {
    if (!this.midiAccess) {
      return [];
    }

    const destinations: MIDIPortInfo[] = [];
    this.midiAccess.outputs.forEach((output) => {
      destinations.push(this.webPortToMIDIPortInfo(output));
    });
    return destinations;
  }

  private webPortToMIDIPortInfo(port: WebMIDIPort): MIDIPortInfo {
    return {
      id: port.id,
      manufacturer: port.manufacturer || null,
      name: port.name || null,
      type: port.type as MIDIPortType,
      version: port.version || null,
      state: port.state as MIDIPortDeviceState,
      connection: port.connection as MIDIPortConnectionState,
    };
  }

  openPort(portId: string): boolean {
    if (!this.midiAccess) {
      return false;
    }

    try {
      const input = this.midiAccess.inputs.get(portId);
      const output = this.midiAccess.outputs.get(portId);

      if (input || output) {
        this.openPorts.add(portId);

        // Set up listener for input ports
        if (input && input.onmidimessage !== undefined) {
          const listener = (event: { data: Uint8Array; timeStamp: number }) => {
            this.eventEmitter.emit({
              portId: input.id,
              data: Array.from(event.data),
              timestamp: event.timeStamp || Date.now(),
            });
          };

          input.onmidimessage = listener;
          this.activeListeners.set(portId, listener);
        }

        return true;
      }
      return false;
    } catch (error) {
      console.error('Failed to open MIDI port:', error);
      return false;
    }
  }

  closePort(portId: string): boolean {
    try {
      // Remove from tracking set
      this.openPorts.delete(portId);

      // Remove the event listener for input ports
      if (this.midiAccess && this.activeListeners.has(portId)) {
        const input = this.midiAccess.inputs.get(portId);
        if (input && input.onmidimessage !== undefined) {
          input.onmidimessage = null;
        }
        this.activeListeners.delete(portId);
      }

      return true;
    } catch (error) {
      console.error('Failed to close MIDI port:', error);
      return false;
    }
  }

  isPortOpen(portId: string): boolean {
    return this.openPorts.has(portId);
  }

  sendMIDIMessage(portId: string, data: Uint8Array): boolean {
    if (!this.midiAccess) {
      return false;
    }

    try {
      const output = this.midiAccess.outputs.get(portId);
      if (output && this.openPorts.has(portId)) {
        output.send(data);
        return true;
      }
      return false;
    } catch (error) {
      console.error('Failed to send MIDI message:', error);
      return false;
    }
  }
}

// Augment global interfaces for TypeScript
declare global {
  interface Window {
    navigator: Navigator;
  }

  interface Navigator {
    requestMIDIAccess?(options?: { sysex?: boolean }): Promise<WebMIDIAccess>;
  }

  // eslint-disable-next-line no-var
  var window: Window;
}

export default new WebMediModule();
