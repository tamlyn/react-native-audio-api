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
  test(): void;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Medi');
