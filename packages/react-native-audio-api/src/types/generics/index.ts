/**
 * This file/directory contains interfaces for nodes and other types that can be
 * used for providing types for each platform and our classes, without further
 * modifications.
 *
 * Same interface/type can be used as:
 *
 * - A type for the native node/host object
 * - A type for the web node abstraction (note: Web Audio API is fully typed, but
 *   this way we can operate on Generic type that isn't really connected with
 *   given implementation)
 * - A type for our user-facing classes
 */

export type { default as IGenericAudioBuffer } from './IGenericAudioBuffer';
export type { default as IGenericAudioDestinationNode } from './IGenericAudioDestinationNode';
export type { default as IGenericAudioNode } from './IGenericAudioNode';
export type { default as IGenericAudioParam } from './IGenericAudioParam';
export type { default as IGenericAudioRecorder } from './IGenericAudioRecorder';
export type { default as IGenericBaseAudioContext } from './IGenericBaseAudioContext';
export type { default as IGenericBiquadFilterNode } from './IGenericBiquadFilterNode';
export type { default as IGenericGainNode } from './IGenericGainNode';
export type { default as IGenericOscillatorNode } from './IGenericOscillatorNode';
export type { default as IGenericPeriodicWave } from './IGenericPeriodicWave';
export type { default as IGenericRecorderAdapterNode } from './IGenericRecorderAdapterNode';
export type { default as IGenericStereoPannerNode } from './IGenericStereoPannerNode';
