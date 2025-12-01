import { StackNavigationProp } from '@react-navigation/stack';
import { icons } from 'lucide-react-native';

import AudioFile from './AudioFile';
import AudioVisualizer from './AudioVisualizer';
import DrumMachine from './DrumMachine';
import Metronome from './Metronome';
import OfflineRendering from './OfflineRendering';
import Oscillator from './Oscillator';
import Piano from './Piano';
import PlaybackSpeed from './PlaybackSpeed/PlaybackSpeed';
import Record from './Record/Record';
import Streaming from './Streaming/Streaming';
import Worklets from './Worklets/Worklets';

type NavigationParamList = {
  Oscillator: undefined;
  Metronome: undefined;
  DrumMachine: undefined;
  Piano: undefined;
  TextToSpeech: undefined;
  AudioFile: undefined;
  PlaybackSpeed: undefined;
  AudioVisualizer: undefined;
  OfflineRendering: undefined;
  // Record: undefined;
  Worklets: undefined;
  Streamer: undefined;
};

export type ExampleKey = keyof NavigationParamList;

export type MainStackProps = StackNavigationProp<NavigationParamList>;
export interface Example {
  key: ExampleKey;
  title: string;
  Icon: React.FC;
  subtitle: string;
  screen: React.FC;
}

export const Examples: Example[] = [
  {
    key: 'DrumMachine',
    title: 'Drum Machine',
    Icon: icons.Drum,
    subtitle: 'Create drum patterns',
    screen: DrumMachine,
  },
  {
    key: 'Piano',
    Icon: icons.Piano,
    title: 'Simple Piano',
    subtitle: 'Play some notes',
    screen: Piano,
  },
  {
    key: 'AudioFile',
    title: 'Audio File',
    Icon: icons.Music,
    subtitle: 'Play an audio file',
    screen: AudioFile,
  },
  {
    key: 'PlaybackSpeed',
    Icon: icons.VenetianMask,
    title: 'Playback Speed',
    subtitle: 'Control playback speed of audio',
    screen: PlaybackSpeed,
  },
  {
    key: 'Metronome',
    title: 'Metronome',
    Icon: icons.Thermometer,
    subtitle: 'Keep time with the beat',
    screen: Metronome,
  },
  {
    key: 'Oscillator',
    title: 'Oscillator',
    Icon: icons.Waves,
    subtitle: 'Generate sound waves',
    screen: Oscillator,
  },
  {
    key: 'AudioVisualizer',
    title: 'Audio Visualizer',
    subtitle: 'Visualize audio data',
    Icon: icons.Activity,
    screen: AudioVisualizer,
  },
  {
    key: 'OfflineRendering',
    title: 'Offline Rendering',
    subtitle: 'Rendering audio in offline',
    Icon: icons.HardDrive,
    screen: OfflineRendering,
  },
  // {
  //   key: 'Record',
  //   title: 'Record',
  //   subtitle: 'Record audio',
  //   screen: Record,
  // },
  {
    key: 'Worklets',
    title: 'Worklets',
    subtitle: 'Process audio on ui thread with worklet support',
    Icon: icons.Code,
    screen: Worklets,
  },
  {
    key: 'Streamer',
    title: 'Streamer',
    subtitle: 'Stream audio from a URL',
    Icon: icons.Radio,
    screen: Streaming,
  },
] as const;
