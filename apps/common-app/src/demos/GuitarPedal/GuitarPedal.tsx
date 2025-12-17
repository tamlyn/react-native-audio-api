import React, { useEffect, useRef, useState } from 'react';
import { View, Text, StyleSheet } from 'react-native';
import { GestureDetector, Gesture } from 'react-native-gesture-handler';
import {
  GainNode,
  WaveShaperNode,
  BiquadFilterNode,
  AudioBuffer,
  AudioBufferSourceNode,
} from 'react-native-audio-api';
import { Container, VerticalSlider } from '../../components';
import { makeDistortionCurve } from './makeDistortionCurve';
import { audioContext } from '../../singletons';

const URL = 'https://files.catbox.moe/xbj6gn.flac';

const MIN_DRIVE_GAIN = 0.05;
const MAX_DRIVE_GAIN = 10;
const MIN_FREQ = 500;
const MAX_FREQ = 20000;

export default function GuitarPedal() {
  const [isActive, setIsActive] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [buffer, setBuffer] = useState<AudioBuffer | null>(null);

  const [drive, setDrive] = useState(0.5);
  const [tone, setTone] = useState(0.5);
  const [level, setLevel] = useState(0.5);

  const sourceNodeRef = useRef<AudioBufferSourceNode | null>(null);
  const driveNodeRef = useRef<GainNode | null>(null);
  const shaperNodeRef = useRef<WaveShaperNode | null>(null);
  const toneNodeRef = useRef<BiquadFilterNode | null>(null);
  const levelNodeRef = useRef<GainNode | null>(null);

  useEffect(() => {
    const init = async () => {
      setIsLoading(true);

      try {
        const audioBuffer = await fetch(URL, {
          headers: {
            'User-Agent':
              'Mozilla/5.0 (Android; Mobile; rv:122.0) Gecko/122.0 Firefox/122.0',
          },
        })
          .then((response) => response.arrayBuffer())
          .then((arrayBuffer) => audioContext.decodeAudioData(arrayBuffer));

        setBuffer(audioBuffer);
      } catch (error) {
        console.error('Error loading audio:', error);
      } finally {
        setIsLoading(false);
      }
    };
    init();

    return () => {
      stopAudio();
    };
  }, []);

  const startAudio = async () => {
    if (!buffer) return;

    if (audioContext.state === 'suspended') {
      await audioContext.resume();
    }

    const source = audioContext.createBufferSource();
    source.buffer = buffer;
    source.onEnded = () => {
      setIsActive(false);
    };

    const driveNode = audioContext.createGain();
    const shaper = audioContext.createWaveShaper();
    const toneNode = audioContext.createBiquadFilter();
    const levelNode = audioContext.createGain();

    shaper.oversample = '4x';
    shaper.curve = makeDistortionCurve(50, audioContext.sampleRate);

    toneNode.type = 'lowpass';
    toneNode.Q.value = 1;

    source.connect(driveNode);
    driveNode.connect(shaper);
    shaper.connect(toneNode);
    toneNode.connect(levelNode);
    levelNode.connect(audioContext.destination);

    source.start();
    source.onEnded = () => {
      setIsActive(false);
    };

    sourceNodeRef.current = source;
    driveNodeRef.current = driveNode;
    shaperNodeRef.current = shaper;
    toneNodeRef.current = toneNode;
    levelNodeRef.current = levelNode;

    updateAudioParams(drive, tone, level);
    setIsActive(true);
  };

  const stopAudio = () => {
    if (sourceNodeRef.current) {
      sourceNodeRef.current.stop();
    }

    sourceNodeRef.current = null;
    driveNodeRef.current = null;
    shaperNodeRef.current = null;
    toneNodeRef.current = null;
    levelNodeRef.current = null;

    setIsActive(false);
  };

  const togglePower = () => {
    if (isLoading || !buffer) return;

    if (isActive) {
      stopAudio();
    } else {
      startAudio();
    }
  };

  const updateAudioParams = (d: number, t: number, l: number) => {
    if (!driveNodeRef.current || !toneNodeRef.current || !levelNodeRef.current)
      return;

    const driveGain = MIN_DRIVE_GAIN + d * (MAX_DRIVE_GAIN - MIN_DRIVE_GAIN);
    driveNodeRef.current.gain.value = driveGain;

    // logarithmic mapping for tone
    const freq = MIN_FREQ * Math.pow(MAX_FREQ / MIN_FREQ, t);
    toneNodeRef.current.frequency.value = freq;

    levelNodeRef.current.gain.value = l;
  };

  useEffect(() => {
    updateAudioParams(drive, tone, level);
  }, [drive, tone, level]);

  return (
    <Container disablePadding>
      <View style={styles.pedalBody}>
        <View style={styles.header}>
          <Text style={styles.brand}>RN AUDIO API</Text>
          <Text style={styles.model}>OVERDRIVE</Text>
        </View>

        <View style={styles.controlsRow}>
          <VerticalSlider
            label="DRIVE"
            value={drive}
            onValueChange={setDrive}
          />
          <VerticalSlider label="TONE" value={tone} onValueChange={setTone} />
          <VerticalSlider
            label="LEVEL"
            value={level}
            onValueChange={setLevel}
          />
        </View>

        <View style={styles.footer}>
          <View style={styles.switchContainer}>
            <View
              style={[
                styles.led,
                {
                  backgroundColor:
                    isActive && !isLoading ? '#ff0000' : '#330000',
                },
              ]}
            />
            <GestureDetector
              gesture={Gesture.Tap()
                .enabled(!isLoading)
                .runOnJS(true)
                .onEnd(togglePower)}
            >
              <View style={[styles.stompSwitch, isLoading && { opacity: 0.5 }]}>
                <View style={styles.stompInner} />
              </View>
            </GestureDetector>
            <Text style={styles.switchLabel}>
              {isLoading ? 'LOADING' : isActive ? 'ON' : 'BYPASS'}
            </Text>
          </View>
        </View>
      </View>
    </Container>
  );
}

const styles = StyleSheet.create({
  pedalBody: {
    flex: 1,
    backgroundColor: '#e6b800',
    margin: 20,
    borderRadius: 20,
    borderWidth: 4,
    borderColor: '#b38f00',
    padding: 20,
    justifyContent: 'space-between',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 10 },
    shadowOpacity: 0.5,
    shadowRadius: 10,
    elevation: 10,
  },
  header: {
    alignItems: 'center',
    marginTop: 20,
  },
  brand: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    letterSpacing: 2,
  },
  model: {
    fontSize: 32,
    fontWeight: '900',
    color: '#000',
    fontStyle: 'italic',
  },
  controlsRow: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    alignItems: 'center',
  },
  footer: {
    alignItems: 'center',
    marginBottom: 40,
  },
  switchContainer: {
    alignItems: 'center',
    gap: 10,
  },
  led: {
    width: 15,
    height: 15,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: '#000',
    marginBottom: 10,
    shadowColor: '#f00',
    shadowOffset: { width: 0, height: 0 },
    shadowOpacity: 0.8,
    shadowRadius: 5,
  },
  stompSwitch: {
    width: 60,
    height: 60,
    borderRadius: 30,
    backgroundColor: '#c0c0c0',
    borderWidth: 2,
    borderColor: '#888',
    justifyContent: 'center',
    alignItems: 'center',
    elevation: 5,
  },
  stompInner: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: '#e0e0e0',
    borderWidth: 1,
    borderColor: '#aaa',
  },
  switchLabel: {
    fontWeight: 'bold',
    color: '#333',
  },
});
