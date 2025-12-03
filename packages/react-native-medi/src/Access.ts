import NativeMedi from './specs/NativeMedi';
import { MIDIInput, MIDIOutput } from './Port';

export class MIDIAccess {
  readonly sysexEnabled: boolean = false;

  get inputs(): MIDIInput[] {
    return NativeMedi.getSources().map((info) => new MIDIInput(info));
  }

  get outputs(): MIDIOutput[] {
    return NativeMedi.getDestinations().map((info) => new MIDIOutput(info));
  }

  private constructor(sysex: boolean) {
    this.sysexEnabled = sysex;
  }

  static async create(sysex: boolean): Promise<MIDIAccess> {
    await NativeMedi.prepareMIDIClient(sysex);
    return new MIDIAccess(sysex);
  }
}

export async function requestMIDIAccess(
  sysex: boolean = false
): Promise<MIDIAccess> {
  return await MIDIAccess.create(sysex);
}
