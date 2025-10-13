import React, { useState } from 'react';
import { View, Text, TouchableOpacity, TextInput } from 'react-native';
import { multiply } from 'react-native-medi';

const Medi: React.FC = () => {
  const [numberA, setNumberA] = useState('5');
  const [numberB, setNumberB] = useState('3');
  const [result, setResult] = useState<number | null>(null);

  const handleMultiply = () => {
    const a = parseFloat(numberA);
    const b = parseFloat(numberB);

    if (!isNaN(a) && !isNaN(b)) {
      const multiplyResult = multiply(a, b);
      setResult(multiplyResult);
    }
  };

  return (
    <View style={{ padding: 20, backgroundColor: '#f5f5f5', flex: 1 }}>
      <Text style={{ fontSize: 24, fontWeight: 'bold', marginBottom: 20, textAlign: 'center' }}>
        React Native Medi Test
      </Text>

      <View style={{ marginBottom: 20 }}>
        <Text style={{ fontSize: 16, marginBottom: 10 }}>First Number:</Text>
        <TextInput
          style={{
            borderWidth: 1,
            borderColor: '#ccc',
            padding: 10,
            borderRadius: 5,
            backgroundColor: 'white',
            fontSize: 16,
          }}
          value={numberA}
          onChangeText={setNumberA}
          keyboardType="numeric"
          placeholder="Enter first number"
        />
      </View>

      <View style={{ marginBottom: 20 }}>
        <Text style={{ fontSize: 16, marginBottom: 10 }}>Second Number:</Text>
        <TextInput
          style={{
            borderWidth: 1,
            borderColor: '#ccc',
            padding: 10,
            borderRadius: 5,
            backgroundColor: 'white',
            fontSize: 16,
          }}
          value={numberB}
          onChangeText={setNumberB}
          keyboardType="numeric"
          placeholder="Enter second number"
        />
      </View>

      <TouchableOpacity
        style={{
          backgroundColor: '#007AFF',
          padding: 15,
          borderRadius: 5,
          alignItems: 'center',
          marginBottom: 20,
        }}
        onPress={handleMultiply}
      >
        <Text style={{ color: 'white', fontSize: 16, fontWeight: 'bold' }}>
          Multiply using Turbo Module
        </Text>
      </TouchableOpacity>

      {result !== null && (
        <View style={{
          backgroundColor: 'white',
          padding: 20,
          borderRadius: 10,
          alignItems: 'center',
          shadowColor: '#000',
          shadowOffset: { width: 0, height: 2 },
          shadowOpacity: 0.1,
          shadowRadius: 4,
          elevation: 3,
        }}>
          <Text style={{ fontSize: 18, color: '#333' }}>Result:</Text>
          <Text style={{ fontSize: 32, fontWeight: 'bold', color: '#007AFF' }}>
            {numberA} × {numberB} = {result}
          </Text>
        </View>
      )}
    </View>
  );
};

export default Medi;
