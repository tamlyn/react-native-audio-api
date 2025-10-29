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
  const [startedAt, setStartedAt] = useState<number | null>(null);
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

    setStartedAt(Date.now());
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
    if (state !== ExampleState.Recording || !startedAt) {
      return;
    }

    const interval = setInterval(() => {
      const elapsed = Date.now() - startedAt;
      const hours = Math.floor(elapsed / 3600000)
        .toString()
        .padStart(2, '0');
      const minutes = Math.floor((elapsed % 3600000) / 60000)
        .toString()
        .padStart(2, '0');
      const seconds = Math.floor((elapsed % 60000) / 1000)
        .toString()
        .padStart(2, '0');

      durationStringSV.value = `${hours}:${minutes}:${seconds}`;
    }, 1000);

    return () => {
      clearInterval(interval);
    };
  }, [state, startedAt, durationStringSV]);

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

// const Record: FC = () => {
//   // const recorderRef = useRef<AudioRecorder | null>(null);
//   // const aCtxRef = useRef<AudioContext | null>(null);
//   // const recorderAdapterRef = useRef<RecorderAdapterNode | null>(null);
//   // const audioBuffersRef = useRef<AudioBuffer[]>([]);
//   // const sourcesRef = useRef<AudioBufferSourceNode[]>([]);
//   // useEffect(() => {
//   //   const setup = async () => {
//   //     await AudioManager.requestRecordingPermissions();
//   //     recorderRef.current = new AudioRecorder({
//   //       sampleRate: SAMPLE_RATE,
//   //       bufferLengthInSamples: SAMPLE_RATE,
//   //     });
//   //   };
//   //   setup();
//   //   return () => {
//   //     aCtxRef.current?.close();
//   //     stopRecorder();
//   //   };
//   // }, []);
//   // const setupRecording = () => {
//   //   AudioManager.setAudioSessionOptions({
//   //     iosCategory: 'playAndRecord',
//   //     iosMode: 'spokenAudio',
//   //     iosOptions: ['defaultToSpeaker', 'allowBluetoothA2DP'],
//   //   });
//   // };
//   // const stopRecorder = () => {
//   //   if (recorderRef.current) {
//   //     recorderRef.current.stop();
//   //     console.log('Recording stopped');
//   //     // advised, but not required
//   //     AudioManager.setAudioSessionOptions({
//   //       iosCategory: 'playback',
//   //       iosMode: 'default',
//   //     });
//   //   } else {
//   //     console.error('AudioRecorder is not initialized');
//   //   }
//   // };
//   // const startEcho = () => {
//   //   if (!recorderRef.current) {
//   //     console.error('AudioContext or AudioRecorder is not initialized');
//   //     return;
//   //   }
//   //   setupRecording();
//   //   aCtxRef.current = new AudioContext({ sampleRate: SAMPLE_RATE });
//   //   recorderAdapterRef.current = aCtxRef.current.createRecorderAdapter();
//   //   recorderAdapterRef.current.connect(aCtxRef.current.destination);
//   //   recorderRef.current.connect(recorderAdapterRef.current);
//   //   recorderRef.current.start();
//   //   console.log('Recording started');
//   //   console.log('Audio context state:', aCtxRef.current.state);
//   //   if (aCtxRef.current.state === 'suspended') {
//   //     console.log('Resuming audio context');
//   //     aCtxRef.current.resume();
//   //   }
//   // };
//   // /// This stops only the recording, not the audio context
//   // const stopEcho = () => {
//   //   stopRecorder();
//   //   aCtxRef.current = null;
//   //   recorderAdapterRef.current = null;
//   // };
//   // const startRecordReplay = () => {
//   //   if (!recorderRef.current) {
//   //     console.error('AudioRecorder is not initialized');
//   //     return;
//   //   }
//   //   setupRecording();
//   //   audioBuffersRef.current = [];
//   //   recorderRef.current.onAudioReady((event) => {
//   //     const { buffer, numFrames } = event;
//   //     console.log('Audio recorder buffer ready:', buffer.duration, numFrames);
//   //     audioBuffersRef.current.push(buffer);
//   //   });
//   //   recorderRef.current.start();
//   //   setTimeout(() => {
//   //     stopRecorder();
//   //   }, 5000);
//   // };
//   // const stopRecordReplay = () => {
//   //   const aCtx = new AudioContext({ sampleRate: SAMPLE_RATE });
//   //   aCtxRef.current = aCtx;
//   //   if (aCtx.state === 'suspended') {
//   //     aCtx.resume();
//   //   }
//   //   const tNow = aCtx.currentTime;
//   //   let nextStartAt = tNow + 1;
//   //   const buffers = audioBuffersRef.current;
//   //   console.log(tNow, nextStartAt, buffers.length);
//   //   for (let i = 0; i < buffers.length; i++) {
//   //     const source = aCtx.createBufferSource();
//   //     source.buffer = buffers[i];
//   //     source.connect(aCtx.destination);
//   //     sourcesRef.current.push(source);
//   //     source.start(nextStartAt);
//   //     nextStartAt += buffers[i].duration;
//   //   }
//   //   setTimeout(
//   //     () => {
//   //       console.log('clearing data');
//   //       audioBuffersRef.current = [];
//   //       sourcesRef.current = [];
//   //     },
//   //     (nextStartAt - tNow) * 1000
//   //   );
//   // };
//   // return (
//   //   <Container style={{ gap: 40 }}>
// <Text style={{ color: colors.gray, fontSize: 18, textAlign: 'center' }}>
//   Sample rate: {SAMPLE_RATE}
// </Text>
//   //     <View style={{ alignItems: 'center', gap: 10, paddingTop: 20 }}>
//   //       <Text style={{ color: colors.white, fontSize: 16 }}>Echo</Text>
//   //       <Button title="Start Recording" onPress={startEcho} />
//   //       <Button title="Stop Recording" onPress={stopEcho} />
//   //     </View>
//   //     <View style={{ alignItems: 'center', gap: 10, paddingTop: 40 }}>
//   //       <Text style={{ color: colors.white, fontSize: 16 }}>
//   //         Record & replay
//   //       </Text>
//   //       <Button title="Record for Replay" onPress={startRecordReplay} />
//   //       <Button title="Replay" onPress={stopRecordReplay} />
//   //     </View>
//   //   </Container>
//   // );
// };
