// Export native module interface
export { default as NativeMedi } from './NativeMedi.web';
export type {
  MIDIPortInfo,
  MIDIMessage,
  MIDIPortDeviceState,
  MIDIPortConnectionState,
  MIDIPortType,
} from './types';

// Export Port classes
export { MIDIPort, MIDIInput, MIDIOutput } from './web/Port';

// Export Access class and function
export { MIDIAccess, requestMIDIAccess } from './web/Access';
