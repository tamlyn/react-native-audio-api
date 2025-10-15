// MIDI Port Types
export type MIDIPortType = 'input' | 'output';
export type MIDIPortDeviceState = 'connected' | 'disconnected';
export type MIDIPortConnectionState = 'open' | 'closed' | 'pending';

// Base Event Types
export interface EventInit {
  bubbles?: boolean;
  cancelable?: boolean;
  composed?: boolean;
}

export interface Event {
  readonly type: string;
  readonly bubbles: boolean;
  readonly cancelable: boolean;
  readonly composed: boolean;
  readonly currentTarget: EventTarget | null;
  readonly target: EventTarget | null;
  readonly timeStamp: number;
  preventDefault(): void;
  stopPropagation(): void;
  stopImmediatePropagation(): void;
}

export interface EventTarget {
  addEventListener(
    type: string,
    listener: EventListenerOrEventListenerObject | null,
    options?: boolean | AddEventListenerOptions
  ): void;
  removeEventListener(
    type: string,
    listener: EventListenerOrEventListenerObject | null,
    options?: boolean | EventListenerOptions
  ): void;
  dispatchEvent(event: Event): boolean;
}

export interface EventListenerOrEventListenerObject {
  (evt: Event): void;
}

export interface AddEventListenerOptions extends EventListenerOptions {
  once?: boolean;
  passive?: boolean;
}

export interface EventListenerOptions {
  capture?: boolean;
}

// MIDI Events
export interface MIDIMessageEventInit extends EventInit {
  data: Uint8Array;
}

export interface MIDIConnectionEventInit extends EventInit {
  port: MIDIPort;
}

export interface MIDIMessageEvent extends Event {
  readonly data: Uint8Array;
}

export interface MIDIConnectionEvent extends Event {
  readonly port: MIDIPort;
}

// MIDI Port Interface
export interface MIDIPort {
  readonly id: string;
  readonly manufacturer: string | null;
  readonly name: string | null;
  readonly type: MIDIPortType;
  readonly version: string | null;
  readonly state: MIDIPortDeviceState;
  readonly connection: MIDIPortConnectionState;
  onstatechange: ((this: MIDIPort, ev: MIDIConnectionEvent) => unknown) | null;
  open(): Promise<MIDIPort>;
  close(): Promise<MIDIPort>;
}

// MIDI Input Interface
export interface MIDIInput extends MIDIPort {
  readonly type: 'input';
  onmidimessage: ((this: MIDIInput, ev: MIDIMessageEvent) => unknown) | null;
}

// MIDI Output Interface
export interface MIDIOutput extends MIDIPort {
  readonly type: 'output';
  send(data: Uint8Array | number[], timestamp?: number): void;
  clear(): void;
}

// MIDI Maps
export interface MIDIInputMap extends Map<string, MIDIInput> {}
export interface MIDIOutputMap extends Map<string, MIDIOutput> {}

// MIDI Access Interface
export interface MIDIAccess {
  readonly inputs: MIDIInputMap;
  readonly outputs: MIDIOutputMap;
  onstatechange:
    | ((this: MIDIAccess, ev: MIDIConnectionEvent) => unknown)
    | null;
  readonly sysexEnabled: boolean;
}

// MIDI Access Options
export interface MIDIOptions {
  sysex?: boolean;
}

// Main entry point function
export declare function requestMIDIAccess(
  options?: MIDIOptions
): Promise<MIDIAccess>;
