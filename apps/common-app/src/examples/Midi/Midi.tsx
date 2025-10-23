import React from 'react';
import { NativeMedi } from '../../../../../packages/react-native-medi/src';
import { Button, View } from 'react-native';

const Medi: React.FC = () => {
  const testCallback = () => {
    console.log('Calling NativeMedi.test()');
    NativeMedi.test();
  };

  return (
    <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center' }}>
      <Button title="Test" onPress={testCallback} />
      <Button title="Prepare MIDI Client" onPress={() => NativeMedi.prepareMIDIClient()} />
      <Button
        title="Get MIDI Sources"
        onPress={() => {
          const sources = NativeMedi.getSources();
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
    </View>
  );
};

export default Medi;
