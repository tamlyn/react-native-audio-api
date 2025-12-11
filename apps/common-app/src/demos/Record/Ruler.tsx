import { Group, Path, Skia, SkSize } from '@shopify/react-native-skia';
import React, { useEffect, useMemo } from 'react';
import {
  cancelAnimation,
  Easing,
  useDerivedValue,
  useSharedValue,
  withRepeat,
  withTiming,
} from 'react-native-reanimated';

import constants from './constants';
import { RecordingState } from './types';

interface InfiniteRulerProps {
  canvasSize: SkSize;
  state: RecordingState;
}

const InfiniteRuler: React.FC<InfiniteRulerProps> = (props) => {
  const translateX = useSharedValue(0);

  const { state, canvasSize } = props;
  const { width, height } = canvasSize;
  const totalTicks = Math.ceil(width / constants.pixelsPerSecond) + 6;

  const gridPath = useMemo(() => {
    const path = Skia.Path.Make();
    const dist = constants.pixelsPerSecond;

    const initialShift =
      (width / constants.pixelsPerSecond -
        Math.floor(width / constants.pixelsPerSecond)) *
        constants.pixelsPerSecond -
      1;

    for (let i = 0; i < totalTicks; i++) {
      const x = i * dist + initialShift;
      path.addRect({ x: x, y: height - 15, width: 1, height: 15 });
    }
    return path;
  }, [height, totalTicks, width]);

  useEffect(() => {
    if (state === RecordingState.Recording) {
      translateX.value = withRepeat(
        withTiming(translateX.value - constants.pixelsPerSecond, {
          duration: 1000,
          easing: Easing.linear,
        }),
        -1, // Infinite loop
        false // Do not reverse
      );
    } else {
      cancelAnimation(translateX);
      translateX.value = translateX.value % constants.pixelsPerSecond;
    }
  }, [state, translateX]);

  const transform = useDerivedValue(() => [{ translateX: translateX.value }]);

  return (
    <Group transform={transform}>
      <Path path={gridPath} color="rgba(255,255,255,0.3)" />
    </Group>
  );
};

export default InfiniteRuler;
