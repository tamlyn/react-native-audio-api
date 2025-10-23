import React from 'react';
import { NativeMedi } from '../../../../../packages/react-native-medi/src';
import { Button, View } from 'react-native';

const Medi: React.FC = () => {
  const [sourcePort, setSourcePort] = React.useState("");
  const testCallback = () => {
    console.log('Calling NativeMedi.test()');
    NativeMedi.test();
  };

  return (
    <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center' }}>
      <Button title="Test" onPress={testCallback} />
      <Button title="Prepare MIDI Client" onPress={() => NativeMedi.prepareMIDIClient(false)} />
      <Button
        title="Get MIDI Sources"
        onPress={() => {
          const sources = NativeMedi.getSources();
          setSourcePort(sources[0]?.id || "");
          console.log('MIDI Sources:', sources);
        }}
      />
      <Button
        title="Get MIDI Destinations"
        onPress={() => {
          const destinations = NativeMedi.getDestinations();
          console.log('MIDI Destinations:', destinations);
        }}
      />
      <Button
        title="Open Source Port"
        onPress={() => {
          if (sourcePort) {
            const result = NativeMedi.openPort(sourcePort);
            console.log(`Open Port Result for ${sourcePort}:`, result);
          } else {
            console.log('No source port available to open.');
          }
        }}
      />
      <Button
        title="Close Source Port"
        onPress={() => {
          if (sourcePort) {
            const result = NativeMedi.closePort(sourcePort);
            console.log(`Close Port Result for ${sourcePort}:`, result);
          } else {
            console.log('No source port available to close.');
          }
        }}
      />
    </View>
  );
};

export default Medi;
