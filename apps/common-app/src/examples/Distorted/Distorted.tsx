import React, { useCallback, useEffect, useState, FC } from 'react';
import { ActivityIndicator } from 'react-native';
import {
  AudioContext,
  AudioNode,
  AudioBuffer,
  AudioBufferSourceNode,
} from 'react-native-audio-api';
import { Container, Button } from '../../components';
import { presetEffects } from '../../utils/effects';

const URL = 'https://files.catbox.moe/s2i1wn.flac';

const Distorted: FC = () => {
  const [isPlaying, setIsPlaying] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [buffer, setBuffer] = useState<AudioBuffer | null>(null);

  const aCtxRef = React.useRef<AudioContext | null>(null);
  const effectsMap = React.useRef<Map<string, AudioNode> | null>(null);
  const sourceNodeRef = React.useRef<AudioBufferSourceNode | null>(null);

  const fetchAudioBuffer = useCallback(async () => {
    setIsLoading(true);

    if (!aCtxRef.current) {
      aCtxRef.current = new AudioContext();
    }
    const audioContext = aCtxRef.current;

    effectsMap.current = presetEffects.distorted(audioContext);

    const audioBuffer = await fetch(URL, {
      headers: {
        'User-Agent':
          'Mozilla/5.0 (Android; Mobile; rv:122.0) Gecko/122.0 Firefox/122.0',
      },
    })
      .then((response) => response.arrayBuffer())
      .then((arrayBuffer) => audioContext.decodeAudioData(arrayBuffer))
      .catch((error) => {
        console.error('Error decoding audio data source:', error);
        return null;
      });

    setBuffer(audioBuffer);

    setIsLoading(false);
  }, []);

  const togglePlayPause = useCallback(async () => {
    if (!aCtxRef.current) {
      return;
    }

    if (buffer === null) {
      fetchAudioBuffer();
      return;
    }

    if (isPlaying) {
      sourceNodeRef.current?.stop();
    } else {
      await aCtxRef.current.resume();
      sourceNodeRef.current = aCtxRef.current.createBufferSource();
      sourceNodeRef.current.buffer = buffer;

      let previousNode: AudioNode = sourceNodeRef.current;
      effectsMap.current?.forEach((node) => {
        previousNode.connect(node);
        previousNode = node;
      });

      previousNode.connect(aCtxRef.current.destination);

      sourceNodeRef.current.start();
    }

    setIsPlaying((prev) => !prev);
  }, [isPlaying, buffer, fetchAudioBuffer]);

  useEffect(() => {
    fetchAudioBuffer();

    return () => {
      aCtxRef.current?.close();
      aCtxRef.current = null;
    };
  }, [fetchAudioBuffer]);

  return (
    <Container centered>
      {isLoading && <ActivityIndicator color="#FFFFFF" />}
      <Button
        title={isPlaying ? 'Stop' : 'Play'}
        onPress={togglePlayPause}
        disabled={isLoading}
      />
    </Container>
  );
};

export default Distorted;
