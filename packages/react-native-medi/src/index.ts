import type { EventSubscription } from 'react-native';
import type {
  MIDIPortConnectionState,
  MIDIPortDeviceState,
  MIDIPortInfo,
  MIDIPortType,
} from './types';
import NativeMedi from './NativeMedi';

// Export native module interface
export { default as NativeMedi } from './NativeMedi';
export type {
  MIDIPortInfo,
  MIDIMessage,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIPortType,
} from './types';

export class MIDIPort {
  readonly id: string;
  readonly manufacturer: string | null;
  readonly name: string | null;
  readonly type: MIDIPortType;
  readonly version: string | null;
  readonly state: MIDIPortDeviceState;
  readonly connection: MIDIPortConnectionState;

  constructor(info: MIDIPortInfo) {
    this.id = info.id;
    this.manufacturer = info.manufacturer;
    this.name = info.name;
    this.type = info.type;
    this.version = info.version;
    this.state = info.state;
    this.connection = info.connection;
  }

  open(): boolean {
    return NativeMedi.openPort(this.id);
  }

  close(): boolean {
    return NativeMedi.closePort(this.id);
  }
}

export class MIDIOutput extends MIDIPort {}

export class MIDIInput extends MIDIPort {
  private eventSubscription: null | EventSubscription = null;

  get onmidimessage(): null | EventSubscription {
    return this.eventSubscription;
  }

  set onmidimessage(handler: (data: Uint8Array) => void) {
    if (this.eventSubscription) {
      this.eventSubscription.remove();
    }
    this.eventSubscription = NativeMedi.onMidiMessage((message) => {
      if (message.portId === this.id) {
        handler(new Uint8Array(message.data));
      }
    });
  }
}

export class MIDIAccess {
  readonly sysexEnabled: boolean = false;

  get inputs(): MIDIInput[] {
    return NativeMedi.getSources().map((info) => new MIDIInput(info));
  }

  get outputs(): MIDIOutput[] {
    return NativeMedi.getDestinations().map((info) => new MIDIOutput(info));
  }

  constructor(sysex: boolean) {
    NativeMedi.prepareMIDIClient(sysex);
    this.sysexEnabled = sysex;
  }
}

export function requestMIDIAccess(sysex: boolean = false): Promise<MIDIAccess> {
  return Promise.resolve(new MIDIAccess(sysex));
}
