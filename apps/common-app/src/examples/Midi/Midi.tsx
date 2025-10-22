import React from 'react';
import { NativeMedi } from '../../../../../packages/react-native-medi/src';
import { Button, View } from 'react-native';

const Medi: React.FC = () => {
  const testCallback = () => {
    NativeMedi.test();
  };

  return (
    <View style={{ flex: 1, padding: 20 }}>
      <Button title="Test" onPress={testCallback} />
    </View>
  );
};

export default Medi;
