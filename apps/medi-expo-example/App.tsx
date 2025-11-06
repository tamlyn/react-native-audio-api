import React, { JSX } from 'react';
import { StatusBar } from 'expo-status-bar';
import { MIDIInput, requestMIDIAccess } from 'react-native-medi';
import { Button, View, Text, ScrollView, StyleSheet } from 'react-native';

function getColoredMidiMessage(data: Uint8Array): JSX.Element {
  // Map MIDI message types to colors
  // each message is 3 bytes: [status, data1, data2]
  // statuses:
  // 128, 135 - Note Off (red)
  // 144, 151 - Note On (green)
  // 176, 183 - Control Change (dark purple)
  // others - default color
  const status = data[0];
  const RED = '#FF5555';
  const GREEN = '#55FF55';
  const DARK_PURPLE = '#AA00AA';
  const DEFAULT_COLOR = '#AAAAAA'; // gray

  let color;
  if (status >= 128 && status <= 135) {
    color = RED;
  } else if (status >= 144 && status <= 151) {
    color = GREEN;
  } else if (status >= 176 && status <= 183) {
    color = DARK_PURPLE;
  } else {
    color = DEFAULT_COLOR;
  }

  return <Text style={{ color }}>{`${Array.from(data).join(', ')}`}</Text>;
}

export default function App() {
  const [sourcePorts, setSourcePorts] = React.useState<MIDIInput[]>([]);
  const [connectedPort, setConnectedPort] = React.useState<MIDIInput | null>(null);
  const [messageLog, setMessageLog] = React.useState<JSX.Element[]>([]);
  const scrollViewRef = React.useRef<ScrollView>(null);

  const scanMIDIDevices = async () => {
    const access = await requestMIDIAccess();
    setSourcePorts(access.inputs);
  };

  const connectToPort = async (portId: string) => {
    if (connectedPort) {
      await connectedPort.close();
    }
    const access = await requestMIDIAccess();
    const input = access.inputs.find((port: MIDIInput) => port.id === portId);
    if (input) {
      await input.open();
      input.onmidimessage = (message: Uint8Array) => {
        setMessageLog((prevLog) => [...prevLog, getColoredMidiMessage(message)]);
      };
      setConnectedPort(input);
      console.log(`Connected to MIDI Input Port: ${portId}`);
    } else {
      console.log(`MIDI Input Port not found: ${portId}`);
      setConnectedPort(null);
    }
  };

  const disconnectPort = async () => {
    if (connectedPort) {
      await connectedPort.close();
      setConnectedPort(null);
      setMessageLog([]);
      console.log(`Disconnected from MIDI Input Port`);
    }
  }

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>React Native MEDI Example</Text>
      </View>
      <View style={styles.content}>
        <View style={styles.buttonContainer}>
          <Button title="Scan MIDI Devices" onPress={scanMIDIDevices} />
          <View style={styles.deviceList}>
            {connectedPort ? (
              <Button title="Disconnect MIDI Device" onPress={disconnectPort} />
            ) : (
              sourcePorts.map((port) => (
                <View key={port.id} style={styles.deviceButton}>
                  <Button title={`Connect to ${port.name}`} onPress={async () => await connectToPort(port.id)} />
                </View>
              ))
            )}
          </View>
        </View>
        <View style={styles.logContainer}>
          <Text style={styles.portName}>Connected Port: {connectedPort ? connectedPort.name : 'None'}</Text>
          <ScrollView
            ref={scrollViewRef}
            style={styles.scrollView}
            onContentSizeChange={() => scrollViewRef.current?.scrollToEnd({ animated: true })}
          >
            {messageLog.map((msg, index) => (
              <View key={index}>
                {msg}
              </View>
            ))}
          </ScrollView>
        </View>
        <View style={styles.clearButton}>
          <Button
            title="Clear Logs"
            onPress={() => {
              setMessageLog([]);
            }}
          />
        </View>
      </View>
      <StatusBar style="auto" />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#1a1a1a',
  },
  header: {
    paddingTop: 60,
    paddingBottom: 20,
    alignItems: 'center',
    backgroundColor: '#2a2a2a',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#fff',
  },
  content: {
    flex: 1,
    alignItems: 'center',
    paddingTop: 20,
  },
  buttonContainer: {
    width: '80%',
  },
  deviceList: {
    marginTop: 10,
    gap: 10,
  },
  deviceButton: {
    marginVertical: 5,
  },
  logContainer: {
    marginTop: 20,
    borderWidth: 1,
    borderColor: '#444',
    padding: 10,
    borderRadius: 5,
    width: '80%',
    height: 300,
    backgroundColor: '#2a2a2a',
  },
  portName: {
    fontWeight: 'bold',
    textAlign: 'center',
    color: '#fff',
  },
  scrollView: {
    width: '100%',
    marginTop: 10,
  },
  clearButton: {
    marginTop: 20,
    width: '80%',
  },
});
