import {
  Canvas,
  Group,
  Path,
  Skia,
  useCanvasRef,
  useCanvasSize,
} from '@shopify/react-native-skia';
import React, { useEffect, useMemo } from 'react';
import { StyleSheet, View } from 'react-native';
import {
  cancelAnimation,
  Easing,
  useDerivedValue,
  useSharedValue,
  withRepeat,
  withTiming,
} from 'react-native-reanimated';

import { audioRecorder as Recorder } from '../../singletons';
import constants from './constants';
import Ruler from './Ruler';
import TimeStream from './TimeStream';
import { RecordingState } from './types';

interface RecordingVisualizationProps {
  state: RecordingState;
}

const RecordingVisualization: React.FC<RecordingVisualizationProps> = ({
  state,
}) => {
  const canvasRef = useCanvasRef();
  // const secondaryCanvasRef = useCanvasRef();

  const { size } = useCanvasSize(canvasRef);
  // const { size: secondaryCanvasSize } = useCanvasSize(secondaryCanvasRef);
  const barHeights = useSharedValue<number[]>([]);
  const translateX = useSharedValue(0);
  const lastIndex = useSharedValue(-1);
  const durationMS = useSharedValue(0);

  const numBars = useMemo(() => {
    if (size.width === 0) {
      return 0;
    }

    return Math.ceil(size.width / (constants.barWidth + constants.barGap)) * 2;
  }, [size.width]);

  const waveformPath = useDerivedValue(() => {
    const path = Skia.Path.Make();
    const currentHeights = barHeights.value;
    const canvasHeight = size.height;

    if (currentHeights.length === 0 || canvasHeight === 0) {
      return path;
    }

    currentHeights.forEach((height, index) => {
      if (height < 0) {
        return;
      }

      const x =
        index * (constants.barWidth + constants.barGap) +
        constants.barWidth / 2;
      const y1 = (canvasHeight - height) / 2;
      const y2 = (canvasHeight + height) / 2;

      path.moveTo(x, y1);
      path.lineTo(x, y2);
    });

    return path;
  }, [size, numBars]);

  // useEffect(() => {
  //   if (numBars <= 0) {
  //     return () => {};
  //   }

  //   if (state !== RecordingState.Recording && state !== RecordingState.Paused) {
  //     barHeights.value = Array(numBars).fill(-1);
  //     return () => {};
  //   }

  //   let lastIndex = -1;
  //   let phase = 0;
  //   const phaseStep = (2 * Math.PI) / (numBars / 4);

  //   const interval = setInterval(() => {
  //     const newBarHeights = [...barHeights.value];

  //     const currentIndex =
  //       newBarHeights.length / 2 -
  //       1 +
  //       Math.floor(-translateX.value / (constants.barWidth + constants.barGap));
  //     const newValue = ((Math.sin(phase) + 1) / 2) * size.height * 0.8;
  //     phase += phaseStep;

  //     newBarHeights[currentIndex] = newValue;

  //     // if (lastIndex !== -1 && lastIndex < currentIndex - 1) {
  //     //   let step =
  //     //     (newValue - newBarHeights[lastIndex]) / (currentIndex - lastIndex);
  //     //   for (let i = lastIndex + 1; i < currentIndex; i++) {
  //     //     newBarHeights[i] = newBarHeights[i - 1] + step;
  //     //   }
  //     // }

  //     if (lastIndex !== -1) {
  //       for (let i = lastIndex + 1; i < currentIndex; i++) {
  //         const interpValue =
  //           newBarHeights[lastIndex] +
  //           ((newValue - newBarHeights[lastIndex]) * (i - lastIndex)) /
  //             (currentIndex - lastIndex);

  //         newBarHeights[i] = interpValue;

  //         if (i > numBars / 2) {
  //           newBarHeights[i - numBars / 2] = interpValue;
  //         }
  //       }
  //     }

  //     newBarHeights[currentIndex] = newValue;
  //     newBarHeights[(currentIndex + 1) % newBarHeights.length] = -1;
  //     newBarHeights[(currentIndex + 2) % newBarHeights.length] = -1;
  //     newBarHeights[(currentIndex + 3) % newBarHeights.length] = -1;
  //     newBarHeights[(currentIndex + 4) % newBarHeights.length] = -1;

  //     if (currentIndex > numBars / 2) {
  //       newBarHeights[currentIndex - numBars / 2] = newValue;
  //     }

  //     lastIndex = currentIndex;
  //     barHeights.value = newBarHeights;
  //   }, constants.updateIntervalMS);

  //   return () => {
  //     clearInterval(interval);
  //   };
  // }, [state, size, barHeights, numBars, translateX]);

  useEffect(() => {
    if (numBars <= 0) {
      return;
    }

    if (barHeights.value.length !== numBars) {
      barHeights.value = new Array(numBars).fill(-1);
    }
  }, [numBars, barHeights]);

  useEffect(() => {
    if (numBars <= 0) {
      return () => {};
    }

    Recorder.onAudioReady(
      {
        sampleRate: constants.sampleRate,
        channelCount: 1,
        bufferLength:
          (constants.updateIntervalMS / 1000.0) * constants.sampleRate,
      },
      (event) => {
        durationMS.value += (event.numFrames / constants.sampleRate) * 1000;
        const { buffer } = event;
        const audioData = buffer.getChannelData(0);

        let maxValue = 0;
        for (let i = 0; i < audioData.length; i++) {
          const val = Math.abs(audioData[i]);
          if (val > maxValue) maxValue = val;
        }

        const db = maxValue > 0 ? 20 * Math.log10(maxValue) : constants.minDb;
        let normalized =
          (db - constants.minDb) / (constants.maxDb - constants.minDb);
        normalized = Math.max(0, Math.min(1, normalized));

        const value = normalized * size.height * 0.8;
        const newBarHeights = [...barHeights.value];

        let currentIndex =
          newBarHeights.length / 2 -
          1 +
          Math.floor(
            -translateX.value / (constants.barWidth + constants.barGap)
          );

        if (newBarHeights[currentIndex] > 0) {
          currentIndex += 1;
        }

        if (lastIndex.value !== -1) {
          for (let i = lastIndex.value + 1; i < currentIndex; i++) {
            const vInter =
              newBarHeights[lastIndex.value] +
              ((value - newBarHeights[lastIndex.value]) *
                (i - lastIndex.value)) /
                (currentIndex - lastIndex.value);

            newBarHeights[i] = vInter;

            if (i > numBars / 2) {
              newBarHeights[i - numBars / 2] = vInter;
            }
          }
        }

        newBarHeights[currentIndex] = value;
        if (currentIndex > numBars / 2) {
          newBarHeights[currentIndex - numBars / 2] = value;
        }

        for (let i = 1; i <= 4; i++) {
          newBarHeights[(currentIndex + i) % newBarHeights.length] = -1;
        }

        lastIndex.value = currentIndex;
        barHeights.value = newBarHeights;
      }
    );

    return () => {
      Recorder.clearOnAudioReady();
    };
  }, [state, size, barHeights, numBars, translateX, lastIndex, durationMS]);

  useEffect(() => {
    if (state === RecordingState.Recording) {
      const animationTarget = -size.width;
      const animationDuration = 1000 * (size.width / constants.pixelsPerSecond);

      translateX.value = withRepeat(
        withTiming(animationTarget, {
          duration: animationDuration,
          easing: Easing.linear,
        }),
        -1, // Infinite loop
        false // No reverse
      );
    } else if (state === RecordingState.Paused) {
      cancelAnimation(translateX);

      const currentIndexOffset =
        -translateX.value / (constants.barWidth + constants.barGap);

      const newBarHeights = [...barHeights.value];

      for (let i = 0; i < newBarHeights.length; i++) {
        newBarHeights[i] =
          newBarHeights[
            (i + Math.floor(currentIndexOffset)) % newBarHeights.length
          ];
      }

      barHeights.value = newBarHeights;
      translateX.value = 0;
    } else {
      cancelAnimation(translateX);
      translateX.value = 0;
      barHeights.value = Array(numBars).fill(-1);
      durationMS.value = 0;
      lastIndex.value = -1;
    }
  }, [state, size, translateX, barHeights, numBars, durationMS, lastIndex]);

  useEffect(() => {
    return () => {
      if (Recorder.isRecording()) {
        Recorder.stop();
      }
    };
  }, []);

  const transformPath = useDerivedValue(() => [
    {
      translateX: translateX.value,
    },
  ]);

  return (
    <>
      <View style={styles.container}>
        <Canvas style={styles.canvas} ref={canvasRef}>
          <Ruler canvasSize={size} state={state} />
          <Group transform={transformPath}>
            <Path
              path={waveformPath}
              style="stroke"
              strokeWidth={constants.barWidth}
              strokeCap="round"
              color="#ff6259"
            />
          </Group>
        </Canvas>
      </View>
      <View style={styles.timeStreamContainer}>
        <TimeStream
          isRecording={state === RecordingState.Recording}
          durationMS={durationMS}
        />
      </View>
    </>
  );
};
export default RecordingVisualization;

const styles = StyleSheet.create({
  container: {
    height: 250,
    width: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.15)',
    flexDirection: 'column',
  },
  canvas: {
    flex: 1,
  },
  secondaryContainer: {
    marginTop: 16,
    height: 75,
    width: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.15)',
    flexDirection: 'column',
  },
  timeStreamContainer: {
    height: 20,
    marginTop: 8,
  },
});
