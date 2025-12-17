import {
  Canvas,
  Path,
  Skia,
  useCanvasRef,
  useCanvasSize,
} from '@shopify/react-native-skia';
import React, { useEffect, useMemo } from 'react';
import { StyleSheet, View } from 'react-native';
import { useDerivedValue, useSharedValue } from 'react-native-reanimated';

import { audioRecorder as Recorder } from '../../singletons';
import { RecordingState } from './types';

interface RecordingVisualizationProps {
  state: RecordingState;
}

const constants = {
  sampleRate: 3125,
  updateIntervalMS: 42,
  barWidth: 2,
  barGap: 2,
  minDb: -40,
  maxDb: 0,
} as const;

class RingBuffer {
  private data: Float64Array;
  private head = 0;
  private count = 0;

  constructor(private readonly capacity: number) {
    this.data = new Float64Array(capacity);
    this.data.fill(-1);
    this.head = capacity - 1;
    this.count = capacity;
  }

  push(v: number) {
    this.head = (this.head + 1) % this.capacity;
    this.data[this.head] = v;

    if (this.count < this.capacity) {
      this.count++;
    }
  }

  toArray(): number[] {
    const out = new Array<number>(this.count);

    for (let i = 0; i < this.count; i++) {
      const idx =
        (this.head - this.count + i + this.capacity + 1) % this.capacity;
      out[i] = this.data[idx];
    }

    return out;
  }

  clear() {
    this.data.fill(-1);
    this.head = 0;
    this.count = 0;
  }

  size() {
    return this.count;
  }
}

const RecordingVisualization: React.FC<RecordingVisualizationProps> = ({
  state,
}) => {
  const canvasRef = useCanvasRef();
  const { size } = useCanvasSize(canvasRef);
  const barHeights = useSharedValue<number[]>([]);

  const numBars = useMemo(() => {
    if (size.width === 0) {
      return 0;
    }

    return Math.floor(size.width / (constants.barWidth + constants.barGap));
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

  useEffect(() => {
    if (numBars <= 0) {
      return;
    }

    const ringBuffer = new RingBuffer(numBars);

    Recorder.onAudioReady(
      {
        sampleRate: constants.sampleRate,
        channelCount: 1,
        bufferLength:
          (constants.updateIntervalMS / 1000.0) * constants.sampleRate,
      },
      (event) => {
        const audioData = event.buffer.getChannelData(0);

        let maxValue = 0;
        for (let i = 0; i < audioData.length; i++) {
          if (Math.abs(audioData[i]) > maxValue) {
            maxValue = Math.abs(audioData[i]);
          }
        }

        const db = maxValue > 0 ? 20 * Math.log10(maxValue) : constants.minDb;
        let normalized =
          (db - constants.minDb) / (constants.maxDb - constants.minDb);
        normalized = Math.max(0, Math.min(1, normalized));

        ringBuffer.push(Math.pow(normalized, 2) * size.height * 0.8);
        barHeights.value = ringBuffer.toArray();
      }
    );

    return () => {
      Recorder.clearOnAudioReady();
    };
  }, [numBars, size.height, barHeights]);

  useEffect(() => {
    if (![RecordingState.Recording, RecordingState.Paused].includes(state)) {
      barHeights.value = Array(numBars).fill(-1);
      return;
    }
  }, [state, numBars, barHeights]);

  return (
    <View style={styles.container}>
      <Canvas style={styles.canvas} ref={canvasRef}>
        <Path
          path={waveformPath}
          style="stroke"
          strokeWidth={constants.barWidth}
          strokeCap="round"
          color="#ff6259"
        />
      </Canvas>
    </View>
  );
};
export default RecordingVisualization;

const styles = StyleSheet.create({
  container: {
    height: 400,
    width: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.15)',
  },
  canvas: {
    flex: 1,
  },
});
