import React, { FC, useCallback, useEffect, useMemo, useState } from 'react';
import {
  AndroidFormat,
  AudioContext,
  AudioManager,
  AudioRecorder,
  BitDepth,
  FileDirectory,
  IOSAudioQuality,
  IOSFormat,
} from 'react-native-audio-api';

import { Text, TextInput, View } from 'react-native';
import Animated, {
  useAnimatedProps,
  useSharedValue,
} from 'react-native-reanimated';
import { Button, Container } from '../../components';
import { colors } from '../../styles';

const SAMPLE_RATE = 48000;

AudioManager.setAudioSessionOptions({
  iosCategory: 'playAndRecord',
  iosMode: 'default',
  iosOptions: ['defaultToSpeaker', 'allowBluetoothA2DP'],
});

const AnimatedTextInput = Animated.createAnimatedComponent(TextInput);

enum ExampleState {
  Idle = 'Idle',
  Recording = 'Recording',
  Paused = 'Paused',
  Playing = 'Playing',
}

const recorder = new AudioRecorder();
const audioContext = new AudioContext({ initSuspended: true });

recorder.enableFileOutput({
  sampleRate: SAMPLE_RATE,
  channels: 2,
  bitRate: 128000,
  bitDepth: BitDepth.Bit32,
  directory: FileDirectory.Document,
  ios: {
    format: IOSFormat.M4A,
    quality: IOSAudioQuality.Medium,
  },
  android: {
    format: AndroidFormat.M4A,
  },
});

const Record: FC = () => {
  const [state, setState] = useState<ExampleState>(ExampleState.Idle);
  const [lastOutput, setLastOutput] = useState<string | null>(null);
  const durationStringSV = useSharedValue('00:00:00:00');

  useEffect(() => {
    recorder.onAudioReady(
      {
        bufferLength: 2048,
        sampleRate: 16000,
        channelCount: 1,
      },
      (event) => {
        console.log(
          'Audio buffer ready:',
          event.buffer.length,
          event.numFrames
        );
      }
    );
  }, []);

  const onStartRecording = useCallback(async () => {
    await AudioManager.setAudioSessionActivity(true);

    const filePath = recorder.start();
    setState(ExampleState.Recording);

    if (filePath) {
      setLastOutput(filePath);
    }
  }, []);

  const onStopRecording = useCallback(async () => {
    setState(ExampleState.Idle);
    const fileInfo = recorder.stop();

    console.log('Recording stopped, file info:', fileInfo);

    await AudioManager.setAudioSessionActivity(false);
  }, []);

  const onPauseRecording = useCallback(() => {
    recorder.pause();
    setState(ExampleState.Paused);
  }, []);

  const onResumeRecording = useCallback(() => {
    recorder.resume();
    setState(ExampleState.Recording);
  }, []);

  const onPlayOutput = useCallback(async () => {
    if (!lastOutput || state !== ExampleState.Idle) {
      return;
    }

    setState(ExampleState.Playing);

    await AudioManager.setAudioSessionActivity(true);

    const buffer = await audioContext.decodeAudioData(lastOutput);
    const source = audioContext.createBufferSource();
    source.buffer = buffer;
    source.connect(audioContext.destination);
    source.start();

    source.onEnded = async () => {
      await audioContext.suspend();
      await AudioManager.setAudioSessionActivity(false);
      setState(ExampleState.Idle);
    };

    if (audioContext.state === 'suspended') {
      await audioContext.resume();
    }
  }, [lastOutput, state]);

  useEffect(() => {
    if (state !== ExampleState.Recording) {
      return;
    }

    const interval = setInterval(() => {
      const elapsedSeconds = recorder.getCurrentDuration();
      console.log('Elapsed seconds:', elapsedSeconds);
      const hours = Math.floor(elapsedSeconds / 3600)
        .toString()
        .padStart(2, '0');
      const minutes = Math.floor((elapsedSeconds % 3600) / 60)
        .toString()
        .padStart(2, '0');
      const seconds = Math.floor(elapsedSeconds % 60)
        .toString()
        .padStart(2, '0');

      durationStringSV.value = `${hours}:${minutes}:${seconds}`;
    }, 1000);

    return () => {
      clearInterval(interval);
    };
  }, [state, durationStringSV]);

  const status = useMemo(() => {
    if (state === ExampleState.Recording) {
      return '🔴 Recording...';
    }

    if (state === ExampleState.Playing) {
      return '🎸 Playing...';
    }

    if (state === ExampleState.Paused) {
      return '⏸️ Paused';
    }

    return '🐷 Idle';
  }, [state]);

  const animatedText = useAnimatedProps(() => {
    return {
      text: durationStringSV.value,
      defaultValue: '00:00:00:00',
    };
  });

  const buttons = useMemo(() => {
    if (state === ExampleState.Recording) {
      return (
        <>
          <Button title="⏸️ Pause" onPress={onPauseRecording} />
          <View style={{ height: 10 }} />
          <Button title="⏹ Stop" onPress={onStopRecording} />
        </>
      );
    }

    if (state === ExampleState.Paused) {
      return (
        <>
          <Button title="▶️ Resume" onPress={onResumeRecording} />
          <View style={{ height: 10 }} />
          <Button title="⏹ Stop" onPress={onStopRecording} />
        </>
      );
    }

    if (state === ExampleState.Playing) {
      return null;
    }

    if (lastOutput) {
      return (
        <>
          <Button title="🔴 Record" onPress={onStartRecording} />
          <View style={{ height: 10 }} />
          <Button title="▶️ Play" onPress={onPlayOutput} />
        </>
      );
    }

    return <Button title="🔴 Record" onPress={onStartRecording} />;
  }, [
    onPlayOutput,
    onStartRecording,
    onStopRecording,
    onPauseRecording,
    onResumeRecording,
    lastOutput,
    state,
  ]);

  return (
    <Container style={{ gap: 40 }}>
      <Text style={{ color: colors.gray, fontSize: 18, textAlign: 'center' }}>
        Sample rate: {SAMPLE_RATE}
      </Text>
      <View style={{ alignItems: 'center', gap: 10, paddingTop: 20 }}>
        <Text style={{ color: colors.white, fontSize: 16 }}>{status}</Text>
        <AnimatedTextInput
          editable={false}
          animatedProps={animatedText}
          style={{
            color: colors.gray,
            fontSize: 48,
            width: '100%',
            textAlign: 'center',
            fontFamily: 'courier-new',
            fontWeight: 'bold',
          }}
        />
        <View style={{ height: 10 }} />
        {buttons}
      </View>
    </Container>
  );
};

export default Record;
