import React, { useRef, useState } from 'react';
import { StyleSheet, Text, TouchableOpacity, View } from 'react-native';
import {
  AudioBuffer,
  AudioBufferSourceNode,
  AudioContext,
} from 'react-native-audio-api';

const Test: React.FC = () => {
  const [status, setStatus] = useState('Ready');
  const [isRunning, setIsRunning] = useState(false);
  const audioContextRef = useRef<AudioContext | null>(null);
  const currentSourceNodeRef = useRef<AudioBufferSourceNode | null>(null);
  const currentNodeRef = useRef<0 | 1>(0); // Track which node is currently playing
  const buffersRef = useRef<AudioBuffer[]>([]);

  const createSourceNode = (
    context: AudioContext,
    buffer: AudioBuffer
  ): AudioBufferSourceNode => {
    const node = context.createBufferSource();
    node.buffer = buffer;
    node.connect(context.destination);

    // Simple callbacks that do nothing (as requested)
    node.onEnded = () => {
      // Do nothing
      console.log('Playback ended');
    };
    node.onPositionChanged = () => {
      // Do nothing
      console.log('Position changed');
    };
    node.onPositionChangedInterval = 127;

    return node;
  };

  const runBugTest = () => {
    if (isRunning) {
      return;
    }

    setIsRunning(true);
    setStatus('Creating AudioContext and buffers...');

    // Create audio context
    if (!audioContextRef.current) {
      audioContextRef.current = new AudioContext();
    }
    const context = audioContextRef.current;

    // Create two separate audio buffers if not exist
    const sampleRate = context.sampleRate;
    const duration = 1; // 1 second
    const frameCount = sampleRate * duration;

    for (let i = 0; i < 2; i++) {
      const buffer = context.createBuffer(2, frameCount, sampleRate);
      for (let channel = 0; channel < buffer.numberOfChannels; channel++) {
        const channelData = new Float32Array(frameCount);
        // Fill with a simple sine wave for testing
        for (let j = 0; j < frameCount; j++) {
          channelData[j] = Math.sin((2 * Math.PI * 440 * j) / sampleRate); // 440 Hz tone
        }
        buffer.copyToChannel(channelData, channel);

        // filling the buffer like this throws during assigning to buffer
        // also the buffer seems to be corrupted and crashes on hot-reload
        // const channelData = buffer.getChannelData(channel);
        // for (let j = 0; j < frameCount; j++) {
        //   channelData[j] = Math.random() * 2 - 1; // Fill with random noise
        // }
      }
      buffersRef.current.push(buffer);
    }

    currentSourceNodeRef.current = createSourceNode(
      context,
      buffersRef.current[0]
    );
    currentSourceNodeRef.current.start();
  };

  function swapNodes() {
    if (!audioContextRef.current) {
      return;
    }

    if (currentSourceNodeRef.current) {
      currentSourceNodeRef.current.stop();
      currentSourceNodeRef.current.onPositionChanged = null;
      currentSourceNodeRef.current.onEnded = null;
      currentSourceNodeRef.current = null;
    }

    currentNodeRef.current = ((currentNodeRef.current + 1) % 2) as 0 | 1;
    currentSourceNodeRef.current = createSourceNode(
      audioContextRef.current,
      buffersRef.current[currentNodeRef.current % 2] // Swap between 0 and 1
    );
    currentSourceNodeRef.current.start();

    setStatus(`Node ${currentNodeRef.current + 1} playing`);
  }

  const stopTest = () => {
    if (currentSourceNodeRef.current) {
      currentSourceNodeRef.current.stop();
      currentSourceNodeRef.current.onPositionChanged = null;
      currentSourceNodeRef.current.onEnded = null;
      currentSourceNodeRef.current = null;
    }

    setIsRunning(false);
    setStatus('Stopped');
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Dual Node Swap Bug Test</Text>

      <Text style={styles.status}>{status}</Text>

      <View style={styles.buttonContainer}>
        <TouchableOpacity
          style={[styles.button, isRunning && styles.buttonDisabled]}
          onPress={runBugTest}
          disabled={isRunning}
        >
          <Text style={styles.buttonText}>
            {isRunning ? 'SWAPPING...' : 'START SWAP TEST'}
          </Text>
        </TouchableOpacity>

        {isRunning && (
          <>
            <TouchableOpacity
              style={[styles.button, styles.stopButton]}
              onPress={swapNodes}
            >
              <Text style={styles.buttonText}>SWAP</Text>
            </TouchableOpacity>
            <TouchableOpacity
              style={[styles.button, styles.stopButton]}
              onPress={stopTest}
            >
              <Text style={styles.buttonText}>STOP</Text>
            </TouchableOpacity>
          </>
        )}
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 20,
    backgroundColor: '#f5f5f5',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 30,
    color: '#333',
  },
  status: {
    fontSize: 16,
    marginBottom: 30,
    color: '#666',
    textAlign: 'center',
  },
  buttonContainer: {
    flexDirection: 'row',
    gap: 15,
  },
  button: {
    backgroundColor: '#007AFF',
    paddingVertical: 15,
    paddingHorizontal: 30,
    borderRadius: 8,
  },
  stopButton: {
    backgroundColor: '#FF3B30',
  },
  buttonDisabled: {
    backgroundColor: '#ccc',
  },
  buttonText: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
  },
});

export default Test;
