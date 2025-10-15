import type {
  MIDIInput,
  MIDIOutput,
  MIDIPort,
  MIDIPortType,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIConnectionEvent,
  MIDIMessageEvent,
} from './types/MIDITypes';

// Base MIDI Port implementation
export abstract class MIDIPortImpl implements MIDIPort {
  readonly id: string;
  readonly manufacturer: string | null;
  readonly name: string | null;
  readonly type: MIDIPortType;
  readonly version: string | null;
  readonly state: MIDIPortDeviceState;
  readonly connection: MIDIPortConnectionState;
  onstatechange: ((this: MIDIPort, ev: MIDIConnectionEvent) => unknown) | null =
    null;

  constructor(
    id: string,
    manufacturer: string | null,
    name: string | null,
    type: MIDIPortType,
    version: string | null,
    state: MIDIPortDeviceState = 'disconnected',
    connection: MIDIPortConnectionState = 'closed'
  ) {
    this.id = id;
    this.manufacturer = manufacturer;
    this.name = name;
    this.type = type;
    this.version = version;
    this.state = state;
    this.connection = connection;
  }

  open(): Promise<MIDIPort> {
    // Native implementation will handle actual opening
    return Promise.resolve(this);
  }

  close(): Promise<MIDIPort> {
    // Native implementation will handle actual closing
    return Promise.resolve(this);
  }
}

// MIDI Input implementation
export class MIDIInputImpl extends MIDIPortImpl implements MIDIInput {
  readonly type = 'input' as const;
  onmidimessage: ((this: MIDIInput, ev: MIDIMessageEvent) => unknown) | null =
    null;

  constructor(
    id: string,
    manufacturer: string | null,
    name: string | null,
    version: string | null,
    state: MIDIPortDeviceState = 'disconnected',
    connection: MIDIPortConnectionState = 'closed'
  ) {
    super(id, manufacturer, name, 'input', version, state, connection);
  }
}

// MIDI Output implementation
export class MIDIOutputImpl extends MIDIPortImpl implements MIDIOutput {
  readonly type = 'output' as const;

  constructor(
    id: string,
    manufacturer: string | null,
    name: string | null,
    version: string | null,
    state: MIDIPortDeviceState = 'disconnected',
    connection: MIDIPortConnectionState = 'closed'
  ) {
    super(id, manufacturer, name, 'output', version, state, connection);
  }

  send(data: Uint8Array | number[], timestamp?: number): void {
    // Native implementation will handle actual sending
    console.log('MIDI send:', data, timestamp);
  }

  clear(): void {
    // Native implementation will handle clearing pending messages
    console.log('MIDI clear');
  }
}
