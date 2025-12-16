import { icons } from 'lucide-react-native';

import Record from './Record/Record';

interface SimplifiedIconProps {
  color?: string;
  size?: number;
}

export interface DemoScreen {
  key: string;
  title: string;
  subtitle: string;
  icon: React.FC<SimplifiedIconProps>;
  screen: React.FC;
}

export const demos: DemoScreen[] = [
  {
    key: 'RecordDemo',
    title: 'Recorder',
    subtitle:
      'Demonstrates microphone permissions, capture, and playback similar to voice memos app.',
    icon: icons.Mic,
    screen: Record,
  },
] as const;
