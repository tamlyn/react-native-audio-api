import React, { useEffect, useRef, useState } from 'react';
import { Dimensions, StyleSheet, View } from 'react-native';
import Animated, {
  cancelAnimation,
  Easing,
  SharedValue,
  useAnimatedStyle,
  useSharedValue,
  withTiming,
} from 'react-native-reanimated';
import constants from './constants';

const SCREEN_WIDTH = Dimensions.get('window').width;

const formatTime = (seconds: number) => {
  const m = Math.floor(seconds / 60);
  const s = seconds % 60;
  return `${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
};

interface TimeStreamProps {
  isRecording: boolean;
  durationMS: SharedValue<number>;
}

function generateInitialTimestamps() {
  const timestamps: number[] = [];

  for (let i = 0; i < 15; i++) {
    timestamps.push(i);
  }

  return timestamps;
}

const TimeStream: React.FC<TimeStreamProps> = ({ isRecording, durationMS }) => {
  const [timestamps, setTimestamps] = useState<number[]>([]);
  const intervalRef = useRef<NodeJS.Timeout | null>(null);

  useEffect(() => {
    if (isRecording) {
      setTimestamps(generateInitialTimestamps());

      intervalRef.current = setInterval(() => {
        const elapsedSeconds = durationMS.value / 1000;
        const futureSecond = Math.ceil(elapsedSeconds + 1);

        setTimestamps((prev) => {
          if (prev.includes(futureSecond)) {
            return prev;
          }

          const cleanList = prev.filter((t) => t > elapsedSeconds - 15);
          return [...cleanList, futureSecond];
        });
      }, 500);
    } else {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
      }
    }

    return () => {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
      }
    };
  }, [isRecording, durationMS]);

  return (
    <View style={StyleSheet.absoluteFill} pointerEvents="none">
      {timestamps.map((seconds) => (
        <FlyingTimestamp
          key={seconds}
          spawnSeconds={seconds}
          durationMS={durationMS}
          isRecording={isRecording}
        />
      ))}
    </View>
  );
};

export default TimeStream;

interface FlyingTimestampProps {
  spawnSeconds: number;
  durationMS: SharedValue<number>;
  isRecording: boolean;
}

const FlyingTimestamp: React.FC<FlyingTimestampProps> = ({
  spawnSeconds,
  durationMS,
  isRecording,
}) => {
  const translateX = useSharedValue(SCREEN_WIDTH);

  const TEXT_WIDTH = 60;
  const VISUAL_NUDGE = 0;

  useEffect(() => {
    if (!isRecording) {
      cancelAnimation(translateX);
      return;
    }

    const elapsedSeconds = Math.floor(durationMS.value / 1000);
    const secondsDiff = spawnSeconds - elapsedSeconds;

    const pixelOffset = secondsDiff * constants.pixelsPerSecond;

    const startX = SCREEN_WIDTH + pixelOffset - TEXT_WIDTH / 2 + VISUAL_NUDGE;

    translateX.value = startX;

    const endX = -100;
    const totalDistance = startX - endX;

    const duration = (totalDistance / constants.pixelsPerSecond) * 1000;

    translateX.value = withTiming(endX, {
      duration: duration,
      easing: Easing.linear,
    });
  }, [spawnSeconds, durationMS, translateX, isRecording]);

  const style = useAnimatedStyle(() => ({
    transform: [{ translateX: translateX.value }],
    position: 'absolute',
    bottom: 0,
    left: 0,
    width: TEXT_WIDTH,
  }));

  return (
    <Animated.Text style={[styles.text, style]}>
      {formatTime(spawnSeconds)}
    </Animated.Text>
  );
};

const styles = StyleSheet.create({
  text: {
    color: '#888',
    fontSize: 16,
    fontFamily: 'Menlo',
    fontWeight: '600',
    textAlign: 'center',
  },
});
