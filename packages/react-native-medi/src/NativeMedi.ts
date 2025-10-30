import type { CodegenTypes, TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import type { MIDIMessage, MIDIPortInfo } from './types';

export interface Spec extends TurboModule {
  prepareMIDIClient(sysex: boolean): void;
  getSources(): MIDIPortInfo[];
  getDestinations(): MIDIPortInfo[];

  openPort(portId: string): boolean;
  closePort(portId: string): boolean;

  readonly onMidiMessage: CodegenTypes.EventEmitter<MIDIMessage>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Medi');
