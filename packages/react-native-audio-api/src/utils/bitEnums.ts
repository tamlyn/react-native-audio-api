function setBitValue(
  output: number,
  value: number,
  shift: number,
  bitWidth = 4
): number {
  const fieldMask = (1 << bitWidth) - 1;

  if ((value & ~fieldMask) !== 0) {
    throw new Error(
      `Value ${value} exceeds the maximum for a field with width ${bitWidth} bits.`
    );
  }

  return (output & ~(fieldMask << shift)) | ((value & fieldMask) << shift);
}

function getBitValue(input: number, shift: number, bitWidth = 4): number {
  const fieldMask = (1 << bitWidth) - 1;
  return (input >> shift) & fieldMask;
}

export function setEnumValue<T extends Record<string, number>>(
  output: number,
  value: T[keyof T],
  shift: number,
  bitWidth = 4
): number {
  return setBitValue(output, value, shift, bitWidth);
}

export function getEnumValue<T extends Record<string, number>>(
  input: number,
  _enum: T,
  shift: number,
  bitWidth = 4
): T[keyof T] {
  return getBitValue(input, shift, bitWidth) as T[keyof T];
}

export function encodeFlags<T extends Record<string, number>>(
  ...flags: Array<T[keyof T]>
): number {
  let output = 0;

  for (let i = 0; i < flags.length; i++) {
    output |= setEnumValue(output, flags[i], i * 4);
  }

  return output;
}
