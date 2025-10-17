import React, { useState, useEffect } from 'react';
import { View, Text, TouchableOpacity, ScrollView, StyleSheet, NativeEventEmitter } from 'react-native';
import { NativeMedi, midiEventManager } from '../../../../../packages/react-native-medi/src';

const Medi: React.FC = () => {
  const [status, setStatus] = useState<string>('Not connected');
  const [deviceInfo, setDeviceInfo] = useState<string>('');
  const [messages, setMessages] = useState<string[]>([]);
  const [currentPortId, setCurrentPortId] = useState<string | null>(null);


  useEffect(() => {
    // Cleanup on unmount
    return () => {
      if (currentPortId) {
        NativeMedi.closePort(currentPortId);
      }
    };
  }, [currentPortId]);

  const connectToFirstDevice = async () => {
    try {
      setStatus('Connecting...');
      setMessages([]);

      // Initialize MIDI
      await NativeMedi.requestMIDIAccess(false);

      // Get input ports
      const inputPorts = await NativeMedi.getInputPorts();

      console.log('Input Ports:', inputPorts);
      // Find first active input
      const activePort = inputPorts.find(port => port.state === 'connected');

      if (!activePort) {
        setStatus('No active devices found');
        setDeviceInfo('Please connect a MIDI device');
        return;
      }

      // Display device info
      setDeviceInfo(
        `${activePort.name || 'Unknown'}\n` +
        `${activePort.manufacturer || 'Unknown manufacturer'}\n` +
        `ID: ${activePort.id}`
      );
      // Setup listener for MIDI messages
      midiEventManager.addMIDIMessageListener(activePort.id, (event: any) => {
        const bytes = Array.from(event.data) as number[];
        const message = `[${bytes.map((b: number) => b.toString(16).padStart(2, '0')).join(' ')}]`;
        console.log('MIDI:', message);
        setMessages(prev => [...prev.slice(-20), message]); // Keep last 20
      });

      // Open the port
      await NativeMedi.openPort(activePort.id);
      setCurrentPortId(activePort.id);
      setStatus('Listening...');

    } catch (error) {
      setStatus('Error: ' + error);
      console.error(error);
    }
  };

  const disconnect = async () => {
    if (currentPortId) {
      await NativeMedi.closePort(currentPortId);
      setCurrentPortId(null);
    }
    setStatus('Disconnected');
    setMessages([]);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>🎹 MIDI Listener</Text>

      <Text style={styles.status}>{status}</Text>

      {deviceInfo && (
        <View style={styles.deviceInfo}>
          <Text style={styles.deviceText}>{deviceInfo}</Text>
        </View>
      )}

      <View style={styles.buttonRow}>
        <TouchableOpacity
          style={[styles.button, styles.connectButton]}
          onPress={connectToFirstDevice}
          disabled={currentPortId !== null}
        >
          <Text style={styles.buttonText}>Connect</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, styles.disconnectButton]}
          onPress={disconnect}
          disabled={currentPortId === null}
        >
          <Text style={styles.buttonText}>Disconnect</Text>
        </TouchableOpacity>
      </View>

      <Text style={styles.label}>MIDI Messages:</Text>
      <ScrollView style={styles.messageBox}>
        {messages.length === 0 ? (
          <Text style={styles.placeholder}>No messages yet...</Text>
        ) : (
          messages.map((msg, index) => (
            <Text key={index} style={styles.message}>{msg}</Text>
          ))
        )}
      </ScrollView>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
    backgroundColor: '#fff',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 20,
  },
  status: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 10,
  },
  deviceInfo: {
    backgroundColor: '#f0f0f0',
    padding: 10,
    borderRadius: 5,
    marginBottom: 20,
  },
  deviceText: {
    fontSize: 14,
  },
  buttonRow: {
    flexDirection: 'row',
    gap: 10,
    marginBottom: 20,
  },
  button: {
    flex: 1,
    padding: 15,
    borderRadius: 5,
    alignItems: 'center',
  },
  connectButton: {
    backgroundColor: '#007AFF',
  },
  disconnectButton: {
    backgroundColor: '#FF3B30',
  },
  buttonText: {
    color: '#fff',
    fontWeight: 'bold',
  },
  label: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 10,
  },
  messageBox: {
    flex: 1,
    backgroundColor: '#000',
    padding: 10,
    borderRadius: 5,
  },
  message: {
    color: '#0f0',
    fontFamily: 'monospace',
    fontSize: 14,
  },
  placeholder: {
    color: '#666',
    textAlign: 'center',
    marginTop: 20,
  },
});

export default Medi;
