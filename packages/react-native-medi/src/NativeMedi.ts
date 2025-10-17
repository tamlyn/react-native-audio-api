import type { TurboModule } from 'react-native';
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

export interface Spec extends TurboModule {
  // Initialize MIDI access
  requestMIDIAccess(sysex: boolean): Promise<boolean>;

  // Device enumeration
  getInputPorts(): Promise<Array<MIDIPortInfo>>;
  getOutputPorts(): Promise<Array<MIDIPortInfo>>;

  // Port operations
  openPort(portId: string): Promise<boolean>;
  closePort(portId: string): Promise<boolean>;

  // Message operations
  sendMIDIMessage(portId: string, data: number[], timestamp?: number): void;
  clearPendingMessages(portId: string): void;

  // Event listener management
  addListener(portId: string): void;
  removeListener(portId: string): void;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Medi');
