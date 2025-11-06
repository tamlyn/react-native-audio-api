# MEDI Expo Example

This is a TypeScript Expo example app that demonstrates the usage of `react-native-medi` package from the workspace.

## Features

This example demonstrates:
- **MIDI Device Scanning**: Find available MIDI input devices
- **Device Connection**: Connect to MIDI input ports
- **Real-time MIDI Messages**: Display incoming MIDI messages with color coding:
  - 🔴 Red: Note Off messages (128-135)
  - 🟢 Green: Note On messages (144-151)
  - 🟣 Dark Purple: Control Change messages (176-183)
  - ⚪ Gray: Other MIDI messages
- **Message Logging**: Scrollable log of all received MIDI data

## Setup

This app is part of the monorepo workspace. Make sure you've installed dependencies from the root:

```bash
# From the root of the monorepo
yarn install
```

## Running the App

```bash
# Start the Expo development server
cd apps/medi-expo-example
yarn start

# Or run directly on a platform
yarn android
yarn ios
yarn web
```

## Technical Details

- **Language**: TypeScript
- **Framework**: Expo SDK 54
- **React Native**: 0.81.5 (different from fabric-example)
- **New Architecture**: Enabled
- **Package Management**: Uses `react-native-medi` from workspace via `workspace:*` protocol

## Notes

- This example uses a different React Native version than `fabric-example` to demonstrate workspace isolation
- The `react-native-medi` package is linked via Yarn workspaces
- Changes to `react-native-medi` will be immediately reflected in this app
- The MIDI example code is adapted from the `common-app` example
