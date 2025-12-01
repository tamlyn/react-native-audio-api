import Record from './Record/Record';

export interface DemoScreen {
  key: string;
  title: string;
  subtitle: string;
  screen: React.FC;
}

export const demos: DemoScreen[] = [
  {
    key: 'RecordDemo',
    title: 'Recording',
    subtitle: 'Record and play audio',
    screen: Record,
  },
] as const;
