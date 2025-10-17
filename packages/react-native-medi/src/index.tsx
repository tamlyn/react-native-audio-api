// Export native module interface
export { default as NativeMedi } from './NativeMedi';
export type { MIDIPortInfo } from './NativeMedi';

// Export event manager for MIDI message listening
export { midiEventManager, MIDIEventManager } from './MIDIEventManager';
export type {
  NativeMIDIMessageEvent,
  NativeStateChangeEvent,
  MIDIMessageEvent,
  StateChangeCallback,
} from './MIDIEventManager';
