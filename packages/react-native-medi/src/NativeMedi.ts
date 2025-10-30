import type { CodegenTypes, TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface MIDIPortInfoCodegen {
  id: string;
  manufacturer: string | null;
  name: string | null;
  type: 'input' | 'output';
  version: string | null;
  state: 'connected' | 'disconnected';
  connection: 'open' | 'closed' | 'pending';
}

export interface MIDIMessageCodegen {
  portId: string;
  data: number[];
  timestamp: number;
}

export interface Spec extends TurboModule {
  prepareMIDIClient(sysex: boolean): Promise<void>;
  getSources(): MIDIPortInfoCodegen[];
  getDestinations(): MIDIPortInfoCodegen[];

  openPort(portId: string): boolean;
  closePort(portId: string): boolean;

  readonly onMidiMessage: CodegenTypes.EventEmitter<MIDIMessageCodegen>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Medi');
