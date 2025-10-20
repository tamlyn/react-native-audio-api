import React, { useRef, useState, useEffect, FC, useMemo } from 'react';
import { Alert, ActivityIndicator } from 'react-native';
import {
  AudioContext,
  AudioBufferSourceNode,
  AudioBuffer,
} from 'react-native-audio-api';

import { Container, Slider, Spacer, Button } from '../../components';

// test url pointing to my public repo, to be changed / deleted
const AUDIO_URL =
  'https://github.com/miloszwielgus/test-files/raw/refs/heads/main/example-true-5-1.ogg';

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

  const {
    context,
    splitter,
    merger,
    gainNode1,
    gainNode2,
    gainNode3,
    gainNode4,
    gainNode5,
    gainNode6,
  } = useMemo(() => {
    const ctx = new AudioContext();

    const splitNode = ctx.createChannelSplitter(6);
    const mergeNode = ctx.createChannelMerger(6);

    const g1 = ctx.createGain();
    g1.gain.value = INITIAL_GAIN;
    const g2 = ctx.createGain();
    g2.gain.value = INITIAL_GAIN;
    const g3 = ctx.createGain();
    g3.gain.value = INITIAL_GAIN;
    const g4 = ctx.createGain();
    g4.gain.value = INITIAL_GAIN;
    const g5 = ctx.createGain();
    g5.gain.value = INITIAL_GAIN;
    const g6 = ctx.createGain();
    g6.gain.value = INITIAL_GAIN;

    splitNode.connect(g1, 0, 0);
    g1.connect(mergeNode, 0, 0);
    splitNode.connect(g2, 1, 0);
    g2.connect(mergeNode, 0, 1);
    splitNode.connect(g3, 2, 0);
    g3.connect(mergeNode, 0, 2);
    splitNode.connect(g4, 3, 0);
    g4.connect(mergeNode, 0, 3);
    splitNode.connect(g5, 4, 0);
    g5.connect(mergeNode, 0, 4);
    splitNode.connect(g6, 5, 0);
    g6.connect(mergeNode, 0, 5);

    mergeNode.connect(ctx.destination);

    return {
      context: ctx,
      splitter: splitNode,
      merger: mergeNode,
      gainNode1: g1,
      gainNode2: g2,
      gainNode3: g3,
      gainNode4: g4,
      gainNode5: g5,
      gainNode6: g6,
    };
  }, []);

  const [audioBuffer, setAudioBuffer] = useState<AudioBuffer | null>(null);

  const sourceNodeRef = useRef<AudioBufferSourceNode | null>(null);

  useEffect(() => {
    const fetchAndDecodeAudio = async () => {
      setIsLoading(true);
      try {
        const response = await fetch(AUDIO_URL);
        const arrayBuffer = await response.arrayBuffer();
        const buffer = await context.decodeAudioData(arrayBuffer);
        setAudioBuffer(buffer);
      } catch (error) {
        console.error('Failed to fetch or decode audio:', error);
        Alert.alert(
          'Error',
          'Could not load the audio file. Check network and URL.'
        );
      } finally {
        setIsLoading(false);
      }
    };

    fetchAndDecodeAudio();

    return () => {
      if (sourceNodeRef.current) {
        sourceNodeRef.current.stop(0);
        sourceNodeRef.current.disconnect();
        sourceNodeRef.current = null;
      }
      context.close();
    };
  }, [context]); 

  const handlePlayPause = () => {
    if (isPlaying) {
      if (sourceNodeRef.current) {
        sourceNodeRef.current.stop(0);
        sourceNodeRef.current.disconnect();
        sourceNodeRef.current = null;
      }
      setIsPlaying(false);
    } else {
      if (!audioBuffer) return;

      const sourceNode = context.createBufferSource();
      sourceNode.buffer = audioBuffer;
      sourceNode.loop = true;

      sourceNode.connect(splitter);

      sourceNode.start(0);
      sourceNodeRef.current = sourceNode;
      setIsPlaying(true);
    }
  };

  const handleGain1Change = (value: number) => {
    setGain1(value);
    gainNode1.gain.value = value;
  };
  const handleGain2Change = (value: number) => {
    setGain2(value);
    gainNode2.gain.value = value;
  };
  const handleGain3Change = (value: number) => {
    setGain3(value);
    gainNode3.gain.value = value;
  };
  const handleGain4Change = (value: number) => {
    setGain4(value);
    gainNode4.gain.value = value;
  };
  const handleGain5Change = (value: number) => {
    setGain5(value);
    gainNode5.gain.value = value;
  };
  const handleGain6Change = (value: number) => {
    setGain6(value);
    gainNode6.gain.value = value;
  };

  return (
    <Container centered>
      {isLoading && <ActivityIndicator size="large" color="#FFFFFF" />}
      <Button
        onPress={handlePlayPause}
        title={isPlaying ? 'Stop' : 'Play'}
        disabled={isLoading || !audioBuffer}
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
