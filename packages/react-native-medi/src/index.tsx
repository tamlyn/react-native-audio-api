import Medi from './NativeMedi';

// Keep multiply method for now so app will build correctly
export function multiply(a: number, b: number): number {
  return Medi.multiply(a, b);
}

// Export MIDI types and interfaces
export type {
  MIDIAccess,
  MIDIInput,
  MIDIOutput,
  MIDIInputMap,
  MIDIOutputMap,
  MIDIPort,
  MIDIPortType,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIOptions,
  MIDIMessageEvent,
  MIDIConnectionEvent,
  MIDIMessageEventInit,
  MIDIConnectionEventInit,
  Event,
  EventInit,
  EventTarget,
} from './types/MIDITypes';

// Export MIDI implementation
export { requestMIDIAccess, MIDIAccessImpl } from './MIDIAccess';
export { MIDIPortImpl, MIDIInputImpl, MIDIOutputImpl } from './MIDIPort';
