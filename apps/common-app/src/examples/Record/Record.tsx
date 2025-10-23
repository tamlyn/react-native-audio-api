import React, { FC, useMemo, useState } from 'react';
import {
  AudioContext,
  AudioManager,
  AudioRecorder,
  BitDepth,
  FileDirectory,
  IOSAudioQuality,
  IOSFormat,
} from 'react-native-audio-api';

import { Text, View } from 'react-native';
import { Button, Container } from '../../components';
import { colors } from '../../styles';

const SAMPLE_RATE = 16000;

AudioManager.setAudioSessionOptions({
  iosCategory: 'playAndRecord',
  iosMode: 'default',
  iosOptions: ['defaultToSpeaker', 'allowBluetoothA2DP'],
});

const Record: FC = () => {
  const [isRecording, setIsRecording] = useState(false);
  const [lastOutput, setLastOutput] = useState<string | null>(null);

  const audioContext = useMemo(() => {
    return new AudioContext({ initSuspended: true });
  }, []);

  const recorder = useMemo(() => {
    const rec = new AudioRecorder();

    rec.enableFileOutput({
      sampleRate: 16000,
      channels: 1,
      bitRate: 32000,
      bitDepth: BitDepth.Bit24,
      directory: FileDirectory.Document,
      ios: {
        format: IOSFormat.M4A,
        quality: IOSAudioQuality.Medium,
      },
      android: {},
    });

    return rec;
  }, []);

  const onStartRecording = async () => {
    await AudioManager.setAudioSessionActivity(true);

    recorder.start();
    setIsRecording(true);
  };

  const onStopRecording = async () => {
    setIsRecording(false);
    const output = recorder.stop();

    await AudioManager.setAudioSessionActivity(false);

    setLastOutput(typeof output === 'string' ? output : null);
  };

  const onPlayOutput = async () => {
    if (!lastOutput) {
      return;
    }

    await AudioManager.setAudioSessionActivity(true);

    const buffer = await audioContext.decodeAudioData(lastOutput);
    const source = audioContext.createBufferSource();
    source.buffer = buffer;
    source.connect(audioContext.destination);
    source.start();

    source.onEnded = async () => {
      await audioContext.suspend();
      await AudioManager.setAudioSessionActivity(false);
    };

    if (audioContext.state === 'suspended') {
      await audioContext.resume();
    }
  };

  return (
    <Container style={{ gap: 40 }}>
      <Text style={{ color: colors.gray, fontSize: 18, textAlign: 'center' }}>
        Sample rate: {SAMPLE_RATE}
      </Text>
      <View style={{ alignItems: 'center', gap: 10, paddingTop: 20 }}>
        <Text style={{ color: colors.white, fontSize: 16 }}>
          {isRecording ? '🔴 Recording...' : '🎙️ Tap to Record'}
        </Text>
        <View style={{ height: 10 }} />
        {isRecording ? (
          <Button title="Stop Recording" onPress={onStopRecording} />
        ) : (
          <Button title="Start Recording" onPress={onStartRecording} />
        )}
        <View style={{ height: 20 }} />
        <Button
          title="Play Last Recording"
          onPress={onPlayOutput}
          disabled={!lastOutput}
        />
      </View>
    </Container>
  );
};

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

export default Record;
