import React, { FC, useEffect, useRef } from 'react';
import {
  AudioBuffer,
  AudioBufferSourceNode,
  AudioContext,
  AudioManager,
  AudioRecorder,
  RecorderAdapterNode,
} from 'react-native-audio-api';

import { Alert, Text, View } from 'react-native';
import { Button, Container } from '../../components';
import { colors } from '../../styles';

const audioRecorder = new AudioRecorder();
const audioContext = new AudioContext({ sampleRate: 44100 });

AudioManager.setAudioSessionOptions({
  iosCategory: 'playAndRecord',
  iosMode: 'default',
  iosOptions: ['defaultToSpeaker', 'allowBluetoothA2DP'],
});

const Record: FC = () => {
  const verifyPermissions = async () => {
    const permissions = await AudioManager.requestRecordingPermissions();

    return permissions === 'Granted';
  };

  const startEcho = async () => {
    const hasPermission = await verifyPermissions();
    if (!hasPermission) {
      Alert.alert(
        'Insufficient permissions!',
        'You need to grant audio recording permissions to use this feature.'
      );
      return;
    }

    const success = await AudioManager.setAudioSessionActivity(true);

    if (!success) {
      Alert.alert(
        'Audio Session Error',
        'Failed to activate audio session for recording.'
      );
      return;
    }

    const adapter = audioContext.createRecorderAdapter();
    adapter.connect(audioContext.destination);
    audioRecorder.connect(adapter);

    let result = audioRecorder.start();

    // if (audioContext.state === 'suspended') {
    result = await audioContext.resume();
    // console.log('Audio context resumed:', result);
    // }

    console.log('Recording started:', result);
  };

  /// This stops only the recording, not the audio context
  const stopEcho = () => {
    audioRecorder.stop();
  };

  useEffect(() => {
    const interval = setInterval(() => {
      console.log(
        'Audio Context time:',
        audioContext?.currentTime,
        'Audio Context state:',
        audioContext?.state,
        'recording duration:',
        audioRecorder.getCurrentDuration()
      );
    }, 200);

    return () => {
      clearInterval(interval);
    };
  }, []);

  return (
    <Container style={{ gap: 40 }}>
      <View style={{ alignItems: 'center', gap: 10, paddingTop: 20 }}>
        <Text style={{ color: colors.white, fontSize: 16 }}>Echo</Text>
        <Button title="Start Recording" onPress={startEcho} />
        <Button title="Stop Recording" onPress={stopEcho} />
      </View>
      {/* <View style={{ alignItems: 'center', gap: 10, paddingTop: 40 }}>
        <Text style={{ color: colors.white, fontSize: 16 }}>
          Record & replay
        </Text>
        <Button title="Record for Replay" onPress={startRecordReplay} />
        <Button title="Replay" onPress={stopRecordReplay} />
      </View> */}
    </Container>
  );
};

export default Record;

// const recorderAdapterRef = useRef<RecorderAdapterNode | null>(null);
// const audioBuffersRef = useRef<AudioBuffer[]>([]);
// const sourcesRef = useRef<AudioBufferSourceNode[]>([]);

// const startEcho = () => {
//   if (!recorderRef.current) {
//     console.error('AudioContext or AudioRecorder is not initialized');
//     return;
//   }
//   setupRecording();

//   aCtxRef.current = new AudioContext({ sampleRate: SAMPLE_RATE });
//   recorderAdapterRef.current = aCtxRef.current.createRecorderAdapter();
//   recorderAdapterRef.current.connect(aCtxRef.current.destination);
//   recorderRef.current.connect(recorderAdapterRef.current);

//   recorderRef.current.start();
//   console.log('Recording started');
//   console.log('Audio context state:', aCtxRef.current.state);
//   if (aCtxRef.current.state === 'suspended') {
//     console.log('Resuming audio context');
//     aCtxRef.current.resume();
//   }
// };

// const startRecordReplay = () => {
//   if (!recorderRef.current) {
//     console.error('AudioRecorder is not initialized');
//     return;
//   }
//   setupRecording();
//   audioBuffersRef.current = [];

//   recorderRef.current.onAudioReady((event) => {
//     const { buffer, numFrames } = event;

//     console.log('Audio recorder buffer ready:', buffer.duration, numFrames);
//     audioBuffersRef.current.push(buffer);
//   });

//   recorderRef.current.start();

//   setTimeout(() => {
//     stopRecorder();
//   }, 5000);
// };

// const stopRecordReplay = () => {
//   const aCtx = new AudioContext({ sampleRate: SAMPLE_RATE });
//   aCtxRef.current = aCtx;

//   if (aCtx.state === 'suspended') {
//     aCtx.resume();
//   }

//   const tNow = aCtx.currentTime;
//   let nextStartAt = tNow + 1;
//   const buffers = audioBuffersRef.current;

//   console.log(tNow, nextStartAt, buffers.length);

//   for (let i = 0; i < buffers.length; i++) {
//     const source = aCtx.createBufferSource();
//     source.buffer = buffers[i];

//     source.connect(aCtx.destination);
//     sourcesRef.current.push(source);

//     source.start(nextStartAt);
//     nextStartAt += buffers[i].duration;
//   }

//   setTimeout(
//     () => {
//       console.log('clearing data');
//       audioBuffersRef.current = [];
//       sourcesRef.current = [];
//     },
//     (nextStartAt - tNow) * 1000
//   );
// };
