import type {
  MIDIPortConnectionState,
  MIDIPortDeviceState,
  MIDIPortInfo,
  MIDIPortType,
} from './types';
import NativeMedi from './NativeMedi.web';

// Export native module interface
export { default as NativeMedi } from './NativeMedi.web';
export type {
  MIDIPortInfo,
  MIDIMessage,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIPortType,
} from './types';

// Web-compatible EventSubscription interface
interface WebEventSubscription {
  remove(): void;
}

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

export class MIDIOutput extends MIDIPort {
  send(data: Uint8Array): Promise<boolean> {
    return Promise.resolve(NativeMedi.sendMIDIMessage(this.id, data));
  }
}

export class MIDIInput extends MIDIPort {
  private eventSubscription: WebEventSubscription | null = null;

  get onmidimessage(): WebEventSubscription | null {
    return this.eventSubscription;
  }

  set onmidimessage(handler: ((data: Uint8Array) => void) | null) {
    if (this.eventSubscription) {
      this.eventSubscription.remove();
      this.eventSubscription = null;
    }

    if (handler) {
      this.eventSubscription = NativeMedi.onMidiMessage((message) => {
        if (message.portId === this.id) {
          handler(new Uint8Array(message.data));
        }
      });
    }
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

  private constructor(sysex: boolean) {
    this.sysexEnabled = sysex;
  }

  static async create(sysex: boolean): Promise<MIDIAccess> {
    await NativeMedi.prepareMIDIClient(sysex);
    return new MIDIAccess(sysex);
  }
}

export async function requestMIDIAccess(
  sysex: boolean = false
): Promise<MIDIAccess> {
  return await MIDIAccess.create(sysex);
}
