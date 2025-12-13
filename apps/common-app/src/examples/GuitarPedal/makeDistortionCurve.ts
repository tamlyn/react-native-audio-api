export function makeDistortionCurve(k: number, sampleRate: number) {
  const curve = new Float32Array(sampleRate);
  const deg = Math.PI / 180;

  for (let i = 0; i < sampleRate; ++i) {
    const x = (i * 2) / sampleRate - 1;
    curve[i] = ((3 + k) * x * 20 * deg) / (Math.PI + k * Math.abs(x));
  }
  return curve;
}
