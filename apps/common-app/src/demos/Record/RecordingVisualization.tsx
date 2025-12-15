import {
  Canvas,
  Group,
  Path,
  Skia,
  useCanvasRef,
  useCanvasSize,
} from '@shopify/react-native-skia';
import React, { useEffect, useMemo } from 'react';
import { Dimensions, StyleSheet, View } from 'react-native';
import {
  cancelAnimation,
  Easing,
  SharedValue,
  useDerivedValue,
  useSharedValue,
  withRepeat,
  withTiming,
} from 'react-native-reanimated';

// import { Spacer } from '../../components';
import { audioRecorder as Recorder } from '../../singletons';
import constants from './constants';
import TimeStream from './TimeStream';
import { RecordingState } from './types';

const { width: windowWidth } = Dimensions.get('window');

const historyNumBars = Math.floor(
  windowWidth / (constants.historyBarWidth + constants.historyBarGap)
);

function getInitialHistory() {
  return new Array(historyNumBars).fill(-1);
}

interface RecordingVisualizationProps {
  state: RecordingState;
}

interface DrawDefaultWaveformParams {
  normalized: number;
  size: { width: number; height: number };
  barHeights: SharedValue<number[]>;
  translateX: SharedValue<number>;
  lastIndex: SharedValue<number>;
  numBars: number;
}

interface DrawHistoryWaveformParams {
  normalized: number;
  lifetimeSize: { width: number; height: number };
  history: SharedValue<number[]>;
  historyHead: SharedValue<number>;
  durationMS: SharedValue<number>;
  historyMidpointMS: SharedValue<number>;
}

function drawDefaultWaveform(params: DrawDefaultWaveformParams) {
  const { normalized, size, barHeights, translateX, lastIndex, numBars } =
    params;

  const value = normalized * size.height * 0.8;
  const newBarHeights = [...barHeights.value];

  let currentIndex =
    newBarHeights.length / 2 -
    1 +
    Math.floor(-translateX.value / (constants.barWidth + constants.barGap));

  newBarHeights[currentIndex] = value;

  if (lastIndex.value !== -1) {
    for (let i = lastIndex.value + 1; i < currentIndex; i++) {
      const vInter =
        newBarHeights[lastIndex.value] +
        ((value - newBarHeights[lastIndex.value]) * (i - lastIndex.value)) /
          (currentIndex - lastIndex.value);

      newBarHeights[i] = vInter;

      if (i > numBars / 2) {
        newBarHeights[i - numBars / 2] = vInter;
      }
    }
  }

  if (currentIndex > numBars / 2) {
    newBarHeights[currentIndex - numBars / 2] = value;
  }

  for (let i = 1; i <= 4; i++) {
    newBarHeights[(currentIndex + i) % newBarHeights.length] = -1;
  }

  lastIndex.value = currentIndex;
  barHeights.value = newBarHeights;
}

function drawHistoryWaveform(params: DrawHistoryWaveformParams) {
  const {
    normalized,
    lifetimeSize,
    historyHead,
    durationMS,
    historyMidpointMS,
  } = params;

  const value = normalized * lifetimeSize.height * 0.8;
  const history = [...params.history.value];
  history[historyHead.value] = value;
  historyHead.value += 1;

  // downsample if needed
  if (historyHead.value >= history.length) {
    const halfLength = history.length / 2;
    for (let i = 0; i < halfLength; i++) {
      history[i] = Math.max(history[2 * i], history[2 * i + 1]);
    }

    historyHead.value = halfLength;
    historyMidpointMS.value = durationMS.value;
  }

  params.history.value = history;
}

// TODO: for tomorrow
// midpoint maxwidth = 80%
// keep rendered waveform history in shared value to compute current max value
const RecordingVisualization: React.FC<RecordingVisualizationProps> = ({
  state,
}) => {
  const canvasRef = useCanvasRef();
  const lifetimeCanvasRef = useCanvasRef();

  const { size } = useCanvasSize(canvasRef);
  const { size: lifetimeSize } = useCanvasSize(lifetimeCanvasRef);
  const barHeights = useSharedValue<number[]>([]);

  const history = useSharedValue<number[]>(getInitialHistory());
  const historyHead = useSharedValue(0);
  const historyMidpointMS = useSharedValue(0);

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

  // const historyWaveformPath = useDerivedValue(() => {
  //   const path = Skia.Path.Make();
  //   // const values = history.value;
  //   // const canvasHeight = lifetimeSize.height;

  //   // if (values.length === 0 || canvasHeight === 0) {
  //   //   return path;
  //   // }

  //   // if (historyHead.value < historyNumBars) {
  //   //   for (let i = 0; i < historyHead.value; i++) {
  //   //     if (values[i] < 0) {
  //   //       continue;
  //   //     }

  //   //     const x =
  //   //       i * (constants.historyBarWidth + constants.historyBarGap) +
  //   //       constants.historyBarWidth / 2;
  //   //     const y1 = (canvasHeight - values[i]) / 2;
  //   //     const y2 = (canvasHeight + values[i]) / 2;

  //   //     path.moveTo(x, y1);
  //   //     path.lineTo(x, y2);
  //   //   }

  //   //   return path;
  //   // }

  //   // const midpointLength = Math.floor(
  //   //   (historyMidpointMS.value / durationMS.value) * historyNumBars
  //   // );
  //   // const historyMidpointStep = history.value.length / 2 / midpointLength;

  //   // for (let i = 0; i < midpointLength; i++) {
  //   //   let maxValue = -1;

  //   //   const startIndex = Math.floor(i * historyMidpointStep);
  //   //   const endIndex = Math.floor((i + 1) * historyMidpointStep);

  //   //   for (let j = startIndex; j < endIndex; j++) {
  //   //     if (values[j] >= maxValue) {
  //   //       maxValue = values[j];
  //   //     }
  //   //   }

  //   //   if (maxValue < 0) {
  //   //     continue;
  //   //   }

  //   //   const x =
  //   //     i * (constants.historyBarWidth + constants.historyBarGap) +
  //   //     constants.historyBarWidth / 2;
  //   //   const y1 = (canvasHeight - maxValue) / 2;
  //   //   const y2 = (canvasHeight + maxValue) / 2;

  //   //   path.moveTo(x, y1);
  //   //   path.lineTo(x, y2);
  //   // }

  //   // const remainingLength = historyNumBars - midpointLength;
  //   // const remainingStep = history.value.length / 2 / remainingLength;

  //   // for (let i = 0; i < remainingLength; i++) {
  //   //   let maxValue = -1;

  //   //   const startIndex = Math.floor(
  //   //     history.value.length / 2 + i * remainingStep
  //   //   );
  //   //   const endIndex = Math.floor(
  //   //     history.value.length / 2 + (i + 1) * remainingStep
  //   //   );

  //   //   for (let j = startIndex; j < endIndex; j++) {
  //   //     if (values[j] >= maxValue) {
  //   //       maxValue = values[j];
  //   //     }
  //   //   }

  //   //   if (maxValue < 0) {
  //   //     continue;
  //   //   }

  //   //   const x =
  //   //     (i + midpointLength) *
  //   //       (constants.historyBarWidth + constants.historyBarGap) +
  //   //     constants.historyBarWidth / 2;
  //   //   const y1 = (canvasHeight - maxValue) / 2;
  //   //   const y2 = (canvasHeight + maxValue) / 2;

  //   //   path.moveTo(x, y1);
  //   //   path.lineTo(x, y2);
  //   // }
  //   // const bucketsInBar = Math.floor(historyHead.value / historyNumBars);

  //   // for (let i = 0; i < historyNumBars; i++) {
  //   //   let maxValue = -1;

  //   //   for (let j = 0; j < bucketsInBar; j++) {
  //   //     const index = i * bucketsInBar + j;
  //   //     if (values[index] > maxValue) {
  //   //       maxValue = values[index];
  //   //     }
  //   //   }

  //   //   if (maxValue < 0) {
  //   //     continue;
  //   //   }

  //   //   const x =
  //   //     i * (constants.historyBarWidth + constants.historyBarGap) +
  //   //     constants.historyBarWidth / 2;
  //   //   const y1 = (canvasHeight - maxValue) / 2;
  //   //   const y2 = (canvasHeight + maxValue) / 2;

  //   //   path.moveTo(x, y1);
  //   //   path.lineTo(x, y2);
  //   // }

  //   return path;
  // }, [lifetimeSize]);

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

        drawDefaultWaveform({
          normalized,
          size,
          barHeights,
          translateX,
          lastIndex,
          numBars,
        });
        drawHistoryWaveform({
          normalized,
          lifetimeSize,
          history,
          historyHead,
          durationMS,
          historyMidpointMS,
        });
      }
    );

    return () => {
      Recorder.clearOnAudioReady();
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [numBars, size, lifetimeSize]);

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
      {/* <Spacer.Vertical size={32} />
      <View style={styles.lifetimeContainer}>
        <Canvas style={styles.canvas} ref={lifetimeCanvasRef}>
          <Group>
            <Path
              path={historyWaveformPath}
              style="stroke"
              strokeWidth={constants.historyBarWidth}
              strokeCap="round"
              color="#ff6259"
            />
          </Group>
        </Canvas>
      </View> */}
    </>
  );
};
export default RecordingVisualization;

const styles = StyleSheet.create({
  container: {
    height: 350,
    width: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.15)',
    flexDirection: 'column',
  },
  canvas: {
    flex: 1,
  },
  timeStreamContainer: {
    height: 20,
    marginTop: 8,
  },
  lifetimeContainer: {
    marginTop: 16,
    height: 75,
    width: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.15)',
    flexDirection: 'column',
  },
});
