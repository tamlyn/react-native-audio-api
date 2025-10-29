import type { CodegenTypes, TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface MIDIPortInfo {
  id: string;
  manufacturer: string | null;
  name: string | null;
  type: 'input' | 'output';
  version: string | null;
  state: 'connected' | 'disconnected'; // 'disconnected' means the device has been unplugged or is otherwise unavailable
  connection: 'open' | 'closed' | 'pending';
}

export interface MIDIMessage {
  portId: string;
  data: number[];
  timestamp: number;
}

export interface Spec extends TurboModule {
  prepareMIDIClient(sysex: boolean): void;
  getSources(): MIDIPortInfo[];
  getDestinations(): MIDIPortInfo[];

  openPort(portId: string): boolean;
  closePort(portId: string): boolean;

  readonly onMidiMessage: CodegenTypes.EventEmitter<MIDIMessage>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Medi');
