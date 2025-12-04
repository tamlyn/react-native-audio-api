import React, { FC, useCallback, useEffect, useState } from 'react';
import { ActivityIndicator, Alert, StyleSheet, View } from 'react-native';
import { AudioManager } from 'react-native-audio-api';
import BackgroundTimer from 'react-native-background-timer';
import { Button, Container, Spacer } from '../../components';
import { colors } from '../../styles';
import AudioPlayer from './AudioPlayer';

const URL =
  'https://software-mansion.github.io/react-native-audio-api/audio/voice/example-voice-01.mp3';

const AudioFile: FC = () => {
  const [isPlaying, setIsPlaying] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [positionPercentage, setPositionPercentage] = useState(0);

  const togglePlayPause = async () => {
    if (isPlaying) {
      await AudioPlayer.pause();
    } else {
      AudioPlayer.setOnPositionChanged((offset) => {
        setPositionPercentage(offset);
      });

      AudioManager.setAudioSessionOptions({
        iosCategory: 'playback',
        iosMode: 'default',
        iosOptions: [],
      });

      const success = await AudioManager.setAudioSessionActivity(true);

      if (!success) {
        Alert.alert(
          'Audio Session Error',
          'Failed to activate audio session for playback.'
        );
        return;
      }

      await AudioPlayer.play();

      AudioManager.observeAudioInterruptions(true);

      AudioManager.getDevicesInfo().then(console.log);
    }

    setIsPlaying((prev) => !prev);
  };

  const fetchAudioBuffer = useCallback(async () => {
    setIsLoading(true);

    await AudioPlayer.loadBuffer(URL);

    setIsLoading(false);
  }, []);

  useEffect(() => {
    AudioManager.setLockScreenInfo({
      title: 'Audio file',
      artist: 'Software Mansion',
      album: 'Audio API',
      duration: 10,
    });

    AudioManager.enableRemoteCommand('remotePlay', true);
    AudioManager.enableRemoteCommand('remotePause', true);
    AudioManager.enableRemoteCommand('remoteSkipForward', true);
    AudioManager.enableRemoteCommand('remoteSkipBackward', true);
    AudioManager.observeAudioInterruptions(true);

    const remotePlaySubscription = AudioManager.addSystemEventListener(
      'remotePlay',
      () => {
        AudioPlayer.play();
        setIsPlaying(true);
      }
    );

    const remotePauseSubscription = AudioManager.addSystemEventListener(
      'remotePause',
      () => {
        AudioPlayer.pause();
        setIsPlaying(false);
      }
    );

    const remoteSkipForwardSubscription = AudioManager.addSystemEventListener(
      'remoteSkipForward',
      (event) => {
        AudioPlayer.seekBy(event.value);
      }
    );

    const remoteSkipBackwardSubscription = AudioManager.addSystemEventListener(
      'remoteSkipBackward',
      (event) => {
        AudioPlayer.seekBy(-event.value);
      }
    );

    const interruptionSubscription = AudioManager.addSystemEventListener(
      'interruption',
      async (event) => {
        if (event.type === 'began') {
          await AudioPlayer.pause();
          setIsPlaying(false);
        } else if (event.type === 'ended' && event.shouldResume) {
          BackgroundTimer.setTimeout(async () => {
            AudioManager.setAudioSessionActivity(true);
            await AudioPlayer.play();
            setIsPlaying(true);
          }, 1000);
        }
      }
    );

    fetchAudioBuffer();

    return () => {
      remotePlaySubscription?.remove();
      remotePauseSubscription?.remove();
      remoteSkipForwardSubscription?.remove();
      remoteSkipBackwardSubscription?.remove();
      interruptionSubscription?.remove();
      AudioManager.resetLockScreenInfo();
      AudioPlayer.reset();
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
      <Spacer.Vertical size={20} />
      <View style={styles.progressContainer}>
        <View
          style={[
            styles.progressBar,
            { width: `${positionPercentage * 100}%` },
          ]}
        />
      </View>
    </Container>
  );
};

export default AudioFile;

const styles = StyleSheet.create({
  progressContainer: {
    width: '100%',
    height: 10,
    backgroundColor: '#333',
    borderRadius: 5,
    overflow: 'hidden',
    marginTop: 20,
  },
  progressBar: {
    height: '100%',
    backgroundColor: colors.main,
    borderRadius: 5,
  },
});
