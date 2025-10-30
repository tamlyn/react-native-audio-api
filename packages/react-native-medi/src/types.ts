export type MIDIPortDeviceState = 'connected' | 'disconnected';

export type MIDIPortConnectionState = 'open' | 'closed' | 'pending';

export type MIDIPortType = 'input' | 'output';

export interface MIDIPortInfo {
  id: string;
  manufacturer: string | null;
  name: string | null;
  type: MIDIPortType;
  version: string | null;
  state: MIDIPortDeviceState;
  connection: MIDIPortConnectionState;
}

export interface MIDIMessage {
  portId: string;
  data: number[];
  timestamp: number;
}
