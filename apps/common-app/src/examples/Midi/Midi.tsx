import React, { JSX } from 'react';
import { MIDIInput, requestMIDIAccess } from 'react-native-medi';
import { Button, View, Text, ScrollView } from 'react-native';
import { Container } from '../../components';
import { colors } from '../../styles';


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

const Medi: React.FC = () => {
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
    const input = access.inputs.find((port) => port.id === portId);
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
    <Container>
      <View style={{ flex: 1, alignItems: 'center' }}>
        <View style={{ width: '80%' }}>
          <Button title="Scan MIDI Devices" onPress={scanMIDIDevices} />
          <View style={{ flexDirection: 'column', gap: 10, marginTop: 10 }}>
            {connectedPort ? (
              <Button title="Disconnect MIDI Device" onPress={disconnectPort} />
            ) : (
              sourcePorts.map((port) => (
                <Button key={port.id} title={`Connect to ${port.name}`} onPress={async () => await connectToPort(port.id)} />
              ))
            )}
          </View>
        </View>
        <View style={{ marginTop: 20, borderWidth: 1, borderColor: '#000', padding: 10, borderRadius: 5, width: '80%', height: 300 }}>
          <Text style={{ fontWeight: 'bold', textAlign: 'center', color: colors.white }}>Connected Port: {connectedPort ? connectedPort.name : 'None'}</Text>
          <ScrollView
            ref={scrollViewRef}
            style={{ width: '100%', marginTop: 10 }}
            onContentSizeChange={() => scrollViewRef.current?.scrollToEnd({ animated: true })}
          >
            {messageLog.map((msg, index) => (
              <View key={index}>
                {msg}
              </View>
            ))}
          </ScrollView>
        </View>
        <View style={{ marginTop: 20, width: '80%' }}>
          <Button
            title="Clear Logs"
            onPress={() => {
              setMessageLog([]);
            }}
          />
        </View>
      </View>

    </Container>
  );
};

export default Medi;
