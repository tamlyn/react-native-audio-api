const constants = {
  sampleRate: 3125,
  updateIntervalMS: 32, //42,
  barWidth: 2,
  barGap: 2,
  minDb: -40,
  maxDb: 0,
  get pixelsPerSecond() {
    return (1000 / this.updateIntervalMS) * (this.barWidth + this.barGap);
  },
  get pixelsPerMS() {
    return this.pixelsPerSecond / 1000;
  },
};

export default constants;
