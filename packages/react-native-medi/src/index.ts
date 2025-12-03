// Export native module interface
export { default as NativeMedi } from './specs/NativeMedi';
export type {
  MIDIPortInfo,
  MIDIMessage,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIPortType,
} from './types';

// Export Port classes
export { MIDIPort, MIDIInput, MIDIOutput } from './Port';

// Export Access class and function
export { MIDIAccess, requestMIDIAccess } from './Access';
