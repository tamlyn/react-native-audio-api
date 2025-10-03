import React, { useRef, useState } from 'react'
import { View, Text, TouchableOpacity, StyleSheet } from 'react-native'
import { AudioContext, AudioBufferSourceNode, AudioBuffer } from 'react-native-audio-api'

const Test: React.FC = () => {
  const [status, setStatus] = useState('Ready')
  const [isRunning, setIsRunning] = useState(false)
  const audioContextRef = useRef<AudioContext | null>(null)
  const buffer1Ref = useRef<AudioBuffer | null>(null)
  const buffer2Ref = useRef<AudioBuffer | null>(null)
  const sourceNode1Ref = useRef<AudioBufferSourceNode | null>(null)
  const sourceNode2Ref = useRef<AudioBufferSourceNode | null>(null)
  const currentNodeRef = useRef<1 | 2>(1) // Track which node is currently playing

  const createSourceNode = (context: AudioContext, buffer: AudioBuffer): AudioBufferSourceNode => {
    const node = context.createBufferSource()
    node.buffer = buffer
    node.connect(context.destination)

    // Simple callbacks that do nothing (as requested)
    node.onEnded = () => {
      // Do nothing
    }
    node.onPositionChanged = () => {
      // Do nothing
    }

    return node
  }

  const runBugTest = async () => {
    if (isRunning) return

    setIsRunning(true)
    setStatus('Creating AudioContext and buffers...')

    try {
      // Create audio context
      if (!audioContextRef.current) {
        audioContextRef.current = new AudioContext()
      }
      const context = audioContextRef.current

      // Create two separate audio buffers if not exist
      if (!buffer1Ref.current || !buffer2Ref.current) {
        const sampleRate = context.sampleRate
        const duration = 3 // 3 seconds
        const frameCount = sampleRate * duration

        // Create buffer 1
        const buffer1 = context.createBuffer(1, frameCount, sampleRate)
        const channelData1 = buffer1.getChannelData(0)
        for (let i = 0; i < frameCount; i++) {
          channelData1[i] = Math.random() * 2 - 1
        }
        buffer1Ref.current = buffer1

        // Create buffer 2
        const buffer2 = context.createBuffer(1, frameCount, sampleRate)
        const channelData2 = buffer2.getChannelData(0)
        for (let i = 0; i < frameCount; i++) {
          channelData2[i] = Math.random() * 2 - 1
        }
        buffer2Ref.current = buffer2
      }

      // Create two AudioBufferSourceNodes connected to destination
      setStatus('Creating two connected AudioBufferSourceNodes...')
      sourceNode1Ref.current = createSourceNode(context, buffer1Ref.current)
      sourceNode2Ref.current = createSourceNode(context, buffer2Ref.current)

      // Start the first node
      setStatus('Starting first node...')
      sourceNode1Ref.current.start()
      currentNodeRef.current = 1

      setStatus('Starting rapid node swapping...')

      setStatus('Starting rapid node swapping...')

      // Rapidly swap between nodes with for loop
      const maxSwaps = 100

      for (let swapCount = 1; swapCount <= maxSwaps; swapCount++) {
        try {
          if (currentNodeRef.current === 1) {
            // Stop node 1, create new node 2 and start it
            if (sourceNode1Ref.current) {
              sourceNode1Ref.current.stop()
            }

            // Create new node 2 from buffer 2 and start it
            sourceNode2Ref.current = createSourceNode(context, buffer2Ref.current!)
            sourceNode2Ref.current.start()
            currentNodeRef.current = 2

          } else {
            // Stop node 2, create new node 1 and start it
            if (sourceNode2Ref.current) {
              sourceNode2Ref.current.stop()
            }

            // Create new node 1 from buffer 1 and start it
            sourceNode1Ref.current = createSourceNode(context, buffer1Ref.current!)
            sourceNode1Ref.current.start()
            currentNodeRef.current = 1
          }

          // Pressure memory with temp objects
          const tempObjects = []
          for (let i = 0; i < 100; i++) {
            tempObjects.push(new Array(1000).fill(Math.random()))
          }

          // Call GC if available
          if (globalThis.gc) {
            globalThis.gc()
          }

          setStatus(`Swap ${swapCount}/${maxSwaps} - Node ${currentNodeRef.current} playing`)

        } catch (error) {
          setStatus(`BUG TRIGGERED: ${error}`)
          setIsRunning(false)
          return
        }
      }

      setStatus('Rapid swapping completed!')
      setIsRunning(false)

    } catch (error) {
      setStatus(`Error: ${error}`)
      setIsRunning(false)
    }
  }

  const stopTest = () => {
    if (sourceNode1Ref.current) {
      sourceNode1Ref.current.stop()
      sourceNode1Ref.current = null
    }
    if (sourceNode2Ref.current) {
      sourceNode2Ref.current.stop()
      sourceNode2Ref.current = null
    }
    setIsRunning(false)
    setStatus('Stopped')
  }

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
          <TouchableOpacity
            style={[styles.button, styles.stopButton]}
            onPress={stopTest}
          >
            <Text style={styles.buttonText}>STOP</Text>
          </TouchableOpacity>
        )}
      </View>
    </View>
  )
}

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
})

export default Test
