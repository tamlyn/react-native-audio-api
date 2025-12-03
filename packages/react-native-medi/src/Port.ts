import type { EventSubscription } from 'react-native';
import type {
  MIDIPortConnectionState,
  MIDIPortDeviceState,
  MIDIPortInfo,
  MIDIPortType,
} from './types';
import NativeMedi from './specs/NativeMedi';

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

  open(): Promise<boolean> {
    return Promise.resolve(NativeMedi.openPort(this.id));
  }

  close(): Promise<boolean> {
    return Promise.resolve(NativeMedi.closePort(this.id));
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
