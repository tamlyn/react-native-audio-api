import { AudioEventEmitter, AudioEventSubscription } from '../events';
import { OnAudioReadyEventType } from '../events/types';
import { IAudioRecorder } from '../interfaces';
import {
  AndroidFormat,
  AudioRecorderCallbackOptions,
  AudioRecorderFileOptions,
  BitDepth,
  FileDirectory,
  IOSAudioQuality,
  IOSFormat,
} from '../types';
import { encodeFlags } from '../utils';
import AudioBuffer from './AudioBuffer';
import RecorderAdapterNode from './RecorderAdapterNode';

function withDefaultOptions(inOptions: AudioRecorderFileOptions) {
  return {
    directory: FileDirectory.Cache,
    sampleRate: 48000,
    channels: 2,
    bitRate: 128000,
    bitDepth: BitDepth.Bit24,
    ...inOptions,
    ios: {
      format: IOSFormat.M4A,
      quality: IOSAudioQuality.High,
      ...(inOptions.ios ?? {}),
    },
    android: {
      format: AndroidFormat.Wav,
      ...(inOptions.android ?? {}),
    },
  };
}

function parseFileOptions(inOptions: AudioRecorderFileOptions) {
  const { sampleRate, channels, bitRate, directory, bitDepth, ios, android } =
    withDefaultOptions(inOptions);

  const iosFlags = encodeFlags(
    ios.format,
    ios.quality,
    ios.flacCompressionLevel || 0,
    directory,
    bitDepth
  );

  // TODO: ensure directory and bitDepth are last in the bitmask
  const androidFlags = encodeFlags(android.format, directory, bitDepth);

  return {
    sampleRate,
    channels,
    bitRate,
    ios: iosFlags,
    android: androidFlags,
  };
}

export default class AudioRecorder {
  protected onAudioReadySubscription: AudioEventSubscription | null = null;
  protected readonly recorder: IAudioRecorder;
  protected options_: AudioRecorderFileOptions | null = null;

  protected readonly audioEventEmitter = new AudioEventEmitter(
    global.AudioEventEmitter
  );

  constructor() {
    this.recorder = global.createAudioRecorder();
  }

  enableFileOutput(options: AudioRecorderFileOptions): void {
    if (this.recorder.isRecording()) {
      throw new Error(
        'Cannot enable file output while recording is in progress. Please stop the recorder before changing output options.'
      );
    }

    this.options_ = options;

    const parsedOptions = parseFileOptions(options);
    this.recorder.enableFileOutput(parsedOptions);
  }

  public get options(): AudioRecorderFileOptions | null {
    return this.options_;
  }

  disableFileOutput(): void {
    this.options_ = null;
    this.recorder.disableFileOutput();
  }

  /** Starts the audio recording process with configured output options */
  public start(): void {
    this.recorder.start();
  }

  /** Stops the audio recording process and releases internal resources */
  public stop(): void | string {
    return this.recorder.stop();
  }

  /** Pauses the audio recording process without tearing down anything */
  public pause(): void {
    this.recorder.pause();
  }

  /** Resumes the audio recording process after being paused */
  public resume(): void {
    this.recorder.resume();
  }

  /**
   * Connects a {@link RecorderAdapterNode} to the recorder’s audio graph.
   *
   * Each node can only be connected once. Attempting to connect a node multiple
   * times will throw an error.
   *
   * @param node - The adapter node to connect to the recorder.
   * @throws If the node has already been connected.
   */
  public connect(node: RecorderAdapterNode): void {
    if (this.recorder.isRecording()) {
      throw new Error(
        'Cannot connect adapter node while recording is in progress. Please stop the recorder before connecting new nodes.'
      );
    }

    if (node.wasConnected) {
      throw new Error(
        'RecorderAdapterNode cannot be connected more than once. Refer to the documentation for more details.'
      );
    }
    node.wasConnected = true;
    this.recorder.connect(node.getNode());
  }

  /**
   * Disconnects the recorder from all connected adapter nodes.
   *
   * After calling this method, any connected {@link RecorderAdapterNode} will no
   * longer receive audio data until reconnected.
   */
  public disconnect(): void {
    this.recorder.disconnect();
  }

  /**
   * Registers a callback to receive raw audio data during an active recording
   * session.
   *
   * The callback is periodically invoked with audio buffers that match the
   * preferred configuration provided in `options`. These parameters (sample
   * rate, buffer length, and channel count) guide how audio data is chunked and
   * delivered, though the exact values may vary depending on device
   * capabilities. Values may vary depending on device capabilities.
   *
   * @param options - Preferred configuration for the audio buffers delivered to
   *   the callback.
   * @param callback - Function invoked each time a new audio buffer is
   *   available. The callback receives an {@link OnAudioReadyEventType} object
   *   containing the audio data and associated metadata.
   */
  public onAudioReady(
    options: AudioRecorderCallbackOptions,
    callback: (event: OnAudioReadyEventType) => void
  ): void {
    if (this.recorder.isRecording()) {
      throw new Error(
        'Cannot set onAudioReady callback while recording is in progress. Please stop the recorder before setting a new callback.'
      );
    }

    if (this.onAudioReadySubscription) {
      this.recorder.clearOnAudioReady();
      this.onAudioReadySubscription.remove();
      this.onAudioReadySubscription = null;
    }

    this.onAudioReadySubscription =
      this.audioEventEmitter.addAudioEventListener('audioReady', (event) => {
        const audioBuffer = new AudioBuffer(event.buffer);
        callback({
          ...event,
          buffer: audioBuffer,
        });
      });

    this.recorder.setOnAudioReady({
      sampleRate: options.sampleRate,
      bufferLength: options.bufferLength,
      channelCount: options.channelCount,
      callbackId: this.onAudioReadySubscription.subscriptionId,
    });
  }

  /**
   * Removes the previously registered audio data callback, if any.
   *
   * This stops further `onAudioReady` events from being delivered during
   * recording. Calling this method is safe even if no callback is currently
   * registered.
   */
  public clearOnAudioReady(): void {
    if (this.recorder.isRecording()) {
      throw new Error(
        'Cannot clear onAudioReady callback while recording is in progress. Please stop the recorder before clearing the callback.'
      );
    }

    if (!this.onAudioReadySubscription) {
      return;
    }

    this.recorder.clearOnAudioReady();

    this.onAudioReadySubscription.remove();
    this.onAudioReadySubscription = null;
  }

  public isRecording(): boolean {
    return this.recorder.isRecording();
  }
}
