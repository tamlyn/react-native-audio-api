/**
 * This file/directory contains interfaces that should be used for defining
 * types and user facing implementation in classes/functions/etc. Exported by
 * this library.
 *
 * This means that underlying native and web implementations have differences.
 */
export type { default as IAnalyserNode } from './IAnalyserNode';
export type {
  default as IAudioBufferBaseSourceNode,
  OnPositionChangedEventCallback,
} from './IAudioBufferBaseSourceNode';
export type { default as IAudioBufferQueueSourceNode } from './IAudioBufferQueueSourceNode';
export type {
  default as IAudioBufferSourceNode,
  LoopEndedEvent,
  LoopEndedEventCallback,
} from './IAudioBufferSourceNode';
export type {
  default as IAudioScheduledSourceNode,
  OnEndedEventCallback,
} from './IAudioScheduledSourceNode';
