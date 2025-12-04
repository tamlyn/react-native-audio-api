# react-native-medi

A React Native library for interacting with MIDI devices. This library provides an API inspired by the Web MIDI API to access and interact with MIDI inputs and outputs on Android and iOS.

## Features

- Request access to MIDI devices.
- List available MIDI input and output ports.
- Open and close MIDI ports.
- Receive MIDI messages from input devices.
- (Coming Soon) Send MIDI messages to output devices.
- (Coming Soon) Live MIDI device connection and disconnection events.

## Installation

```sh
npm install react-native-medi
# or
yarn add react-native-medi
```

### iOS

Run `pod install` in the `ios` directory:

```sh
cd ios && pod install
```

### Android

No additional steps are usually required for Android.

## Usage

### Requesting MIDI Access

To start using MIDI, you need to request access. This returns a `MIDIAccess` object which allows you to list inputs and outputs.

```typescript
import { requestMIDIAccess } from 'react-native-medi';

async function initMidi() {
  try {
    const access = await requestMIDIAccess();
    console.log('MIDI Access granted');
    return access;
  } catch (error) {
    console.error('Failed to request MIDI access', error);
  }
}
```

### Listing Ports

Once you have the `MIDIAccess` object, you can access the `inputs` and `outputs` properties to get a list of available ports.

```typescript
const access = await requestMIDIAccess();

// List Inputs
access.inputs.forEach((input) => {
  console.log(`Input: ${input.name} (ID: ${input.id})`);
});

// List Outputs
access.outputs.forEach((output) => {
  console.log(`Output: ${output.name} (ID: ${output.id})`);
});
```

### Receiving MIDI Messages

To receive messages from a MIDI input device, you need to open the port and set an `onmidimessage` handler.

```typescript
const input = access.inputs[0]; // Select the first input

if (input) {
  await input.open();

  input.onmidimessage = (data: Uint8Array) => {
    console.log('Received MIDI message:', data);
    // data is a Uint8Array containing the MIDI bytes
  };
}
```

### Closing Ports

When you are done with a port, you should close it.

```typescript
await input.close();
```

## API Reference

### `requestMIDIAccess(sysex?: boolean): Promise<MIDIAccess>`

Requests access to the MIDI subsystem.
- `sysex`: (Optional) `boolean` requesting System Exclusive access. Defaults to `false`.

### `MIDIAccess`

- `inputs`: `MIDIInput[]` - Array of available MIDI input ports.
- `outputs`: `MIDIOutput[]` - Array of available MIDI output ports.
- `sysexEnabled`: `boolean` - Whether System Exclusive access is enabled.

### `MIDIPort` (Base class for Input/Output)

- `id`: `string` - Unique identifier for the port.
- `manufacturer`: `string | null`
- `name`: `string | null`
- `type`: `'input' | 'output'`
- `version`: `string | null`
- `state`: `'connected' | 'disconnected'`
- `connection`: `'open' | 'closed' | 'pending'`
- `open(): Promise<boolean>` - Opens the port.
- `close(): Promise<boolean>` - Closes the port.

### `MIDIInput` (extends `MIDIPort`)

- `onmidimessage`: `(data: Uint8Array) => void` - Setter for the event handler that receives MIDI messages.

### `MIDIOutput` (extends `MIDIPort`)

- Currently inherits all properties from `MIDIPort`. Sending data is not yet implemented.

## License

MIT
