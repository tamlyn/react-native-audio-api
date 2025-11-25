import React, { FC, useCallback, useEffect, useMemo, useState } from 'react';
import {
  AudioContext,
  AudioManager,
  AudioRecorder,
} from 'react-native-audio-api';

import { Alert, Text, TextInput, View } from 'react-native';
import Animated, {
  useAnimatedProps,
  useSharedValue,
} from 'react-native-reanimated';
import { Button, Container } from '../../components';
import { colors } from '../../styles';

AudioManager.setAudioSessionOptions({
  iosCategory: 'playAndRecord',
  iosMode: 'default',
  iosOptions: ['defaultToSpeaker', 'allowBluetoothA2DP'],
});

const AnimatedTextInput = Animated.createAnimatedComponent(TextInput);

enum ExampleState {
  Idle = 'Idle',
  Loading = 'Loading',
  Recording = 'Recording',
  Paused = 'Paused',
  Playing = 'Playing',
}

const recorder = new AudioRecorder();
const audioContext = new AudioContext({ initSuspended: true });

recorder.enableFileOutput({
  batchDurationSeconds: 60,
});

const Record: FC = () => {
  const [hasPermissions, setHasPermissions] = useState<boolean>(false);
  const [state, setState] = useState<ExampleState>(ExampleState.Idle);
  const [lastOutput, setLastOutput] = useState<string | null>(null);
  const durationStringSV = useSharedValue('00:00:00');

  useEffect(() => {
    const results = recorder.onAudioReady(
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

    if (results.status === 'error') {
      console.error('Failed to set onAudioReady:', results.message);
    }

    return () => {
      recorder.clearOnAudioReady();
    };
  }, []);

  useEffect(() => {
    recorder.onError((error) => {
      console.error('Recorder error:', error);
      Alert.alert('Recorder Error', error.message);
    });

    return () => {
      recorder.clearOnError();
    };
  }, []);

  const onStartRecording = useCallback(async () => {
    if (state !== ExampleState.Idle) {
      return;
    }

    setState(ExampleState.Loading);

    await AudioManager.setAudioSessionActivity(true);

    if (!hasPermissions) {
      const permissionStatus = await AudioManager.requestRecordingPermissions();

      if (permissionStatus !== 'Granted') {
        Alert.alert(
          'Baka!',
          "Recording permissions are no't granted ahoka gaijin-san"
        );
        return;
      }

      setHasPermissions(true);
    }

    const result = recorder.start();

    if (result.status === 'success') {
      setLastOutput(result.path);
      setState(ExampleState.Recording);
    } else {
      setState(ExampleState.Idle);
      Alert.alert('Error', `Failed to start recording: ${result.message}`);
    }
  }, [hasPermissions, state]);

  const onStopRecording = useCallback(async () => {
    const result = recorder.stop();

    if (result.status === 'error') {
      Alert.alert('Error', `Failed to stop recording: ${result.message}`);
    } else {
      console.log(
        'Recording stopped, file at:',
        result.path,
        'with size:',
        result.size,
        'and duration:',
        result.duration
      );
    }

    await AudioManager.setAudioSessionActivity(false);
    setState(ExampleState.Idle);
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
    (async () => {
      const permissionStatus = await AudioManager.checkRecordingPermissions();

      if (permissionStatus === 'Granted') {
        setHasPermissions(true);
      }
    })();
  }, []);

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
    if (state === ExampleState.Loading) {
      return '⏳ Loading...';
    }

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
      defaultValue: '00:00:00',
    };
  });

  const buttons = useMemo(() => {
    if (state === ExampleState.Loading) {
      return null;
    }

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
