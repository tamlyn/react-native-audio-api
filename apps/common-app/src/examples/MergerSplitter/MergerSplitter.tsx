import React, { useRef, useState, useEffect, FC } from 'react';
import { Alert, ActivityIndicator } from 'react-native';
import {
  AudioContext,
  GainNode,
  AudioBufferSourceNode,
  ChannelSplitterNode,
  ChannelMergerNode,
} from 'react-native-audio-api';

import { Container, Slider, Spacer, Button } from '../../components';

// test url pointing to my public repo, to be changed / deleted
const AUDIO_URL =
  'https://github.com/miloszwielgus/test-files/raw/refs/heads/main/example-music-5-1.ogg';

const INITIAL_GAIN = 0.5;
const labelWidth = 100;

const SplitterMerger: FC = () => {
  const [isPlaying, setIsPlaying] = useState(false);
  const [isLoading, setIsLoading] = useState(false);

  const [gain1, setGain1] = useState(INITIAL_GAIN);
  const [gain2, setGain2] = useState(INITIAL_GAIN);
  const [gain3, setGain3] = useState(INITIAL_GAIN);
  const [gain4, setGain4] = useState(INITIAL_GAIN);
  const [gain5, setGain5] = useState(INITIAL_GAIN);
  const [gain6, setGain6] = useState(INITIAL_GAIN);

  const audioContextRef = useRef<AudioContext | null>(null);
  const sourceNodeRef = useRef<AudioBufferSourceNode | null>(null);
  const splitterRef = useRef<ChannelSplitterNode | null>(null);
  const mergerRef = useRef<ChannelMergerNode | null>(null);
  const audioBufferRef = useRef<AudioBuffer | null>(null);

  const gainNode1Ref = useRef<GainNode | null>(null);
  const gainNode2Ref = useRef<GainNode | null>(null);
  const gainNode3Ref = useRef<GainNode | null>(null);
  const gainNode4Ref = useRef<GainNode | null>(null);
  const gainNode5Ref = useRef<GainNode | null>(null);
  const gainNode6Ref = useRef<GainNode | null>(null);

  const setupAndPlay = async () => {
    const context = audioContextRef.current;
    if (!context) return;

    if (!audioBufferRef.current) {
      setIsLoading(true);
      try {
        const response = await fetch(AUDIO_URL);
        const arrayBuffer = await response.arrayBuffer();
        audioBufferRef.current = await context.decodeAudioData(arrayBuffer);
      } catch (error) {
        console.error('Failed to fetch or decode audio:', error);
        Alert.alert(
          'Error',
          'Could not load the audio file. Check network and URL.'
        );
        setIsLoading(false);
        return;
      } finally {
        setIsLoading(false);
      }
    }

    sourceNodeRef.current = context.createBufferSource();
    sourceNodeRef.current.buffer = audioBufferRef.current;
    sourceNodeRef.current.loop = true;

    splitterRef.current = context.createChannelSplitter(6);
    mergerRef.current = context.createChannelMerger(6);

    gainNode1Ref.current = context.createGain();
    gainNode1Ref.current.gain.value = gain1;
    splitterRef.current.connect(gainNode1Ref.current, 0, 0);
    gainNode1Ref.current.connect(mergerRef.current, 0, 0);

    gainNode2Ref.current = context.createGain();
    gainNode2Ref.current.gain.value = gain2;
    splitterRef.current.connect(gainNode2Ref.current, 1, 0);
    gainNode2Ref.current.connect(mergerRef.current, 0, 1);

    gainNode3Ref.current = context.createGain();
    gainNode3Ref.current.gain.value = gain3;
    splitterRef.current.connect(gainNode3Ref.current, 2, 0);
    gainNode3Ref.current.connect(mergerRef.current, 0, 2);

    gainNode4Ref.current = context.createGain();
    gainNode4Ref.current.gain.value = gain4;
    splitterRef.current.connect(gainNode4Ref.current, 3, 0);
    gainNode4Ref.current.connect(mergerRef.current, 0, 3);

    gainNode5Ref.current = context.createGain();
    gainNode5Ref.current.gain.value = gain5;
    splitterRef.current.connect(gainNode5Ref.current, 4, 0);
    gainNode5Ref.current.connect(mergerRef.current, 0, 4);

    gainNode6Ref.current = context.createGain();
    gainNode6Ref.current.gain.value = gain6;
    splitterRef.current.connect(gainNode6Ref.current, 5, 0);
    gainNode6Ref.current.connect(mergerRef.current, 0, 5);

    sourceNodeRef.current.connect(splitterRef.current);
    mergerRef.current.connect(context.destination);
    sourceNodeRef.current.start(0);
    setIsPlaying(true);
  };

  const stopPlayback = () => {
    if (sourceNodeRef.current) {
      sourceNodeRef.current.stop(0);
      sourceNodeRef.current.disconnect();
      sourceNodeRef.current = null;
    }
    setIsPlaying(false);
  };

  const handlePlayPause = async () => {
    if (isPlaying) {
      stopPlayback();
    } else {
      await setupAndPlay();
    }
  };

  const handleGain1Change = (value: number) => {
    setGain1(value);
    if (gainNode1Ref.current) gainNode1Ref.current.gain.value = value;
  };
  const handleGain2Change = (value: number) => {
    setGain2(value);
    if (gainNode2Ref.current) gainNode2Ref.current.gain.value = value;
  };
  const handleGain3Change = (value: number) => {
    setGain3(value);
    if (gainNode3Ref.current) gainNode3Ref.current.gain.value = value;
  };
  const handleGain4Change = (value: number) => {
    setGain4(value);
    if (gainNode4Ref.current) gainNode4Ref.current.gain.value = value;
  };
  const handleGain5Change = (value: number) => {
    setGain5(value);
    if (gainNode5Ref.current) gainNode5Ref.current.gain.value = value;
  };
  const handleGain6Change = (value: number) => {
    setGain6(value);
    if (gainNode6Ref.current) gainNode6Ref.current.gain.value = value;
  };

  useEffect(() => {
    if (!audioContextRef.current) {
      audioContextRef.current = new AudioContext();
    }
    return () => {
      stopPlayback();
      audioContextRef.current?.close();
    };
  }, []);

  return (
    <Container centered>
      {isLoading && <ActivityIndicator size="large" color="#FFFFFF" />}
      <Button
        onPress={handlePlayPause}
        title={isPlaying ? 'Stop' : 'Play'}
        disabled={isLoading}
      />
      <Spacer.Vertical size={30} />

      <Slider
        label="Gain Path 1"
        value={gain1}
        onValueChange={handleGain1Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={15} />
      <Slider
        label="Gain Path 2"
        value={gain2}
        onValueChange={handleGain2Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={15} />
      <Slider
        label="Gain Path 3"
        value={gain3}
        onValueChange={handleGain3Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={15} />
      <Slider
        label="Gain Path 4"
        value={gain4}
        onValueChange={handleGain4Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={15} />
      <Slider
        label="Gain Path 5"
        value={gain5}
        onValueChange={handleGain5Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={15} />
      <Slider
        label="Gain Path 6"
        value={gain6}
        onValueChange={handleGain6Change}
        min={0.0}
        max={1.5}
        step={0.01}
        minLabelWidth={labelWidth}
      />
      <Spacer.Vertical size={30} />
    </Container>
  );
};

export default SplitterMerger;
