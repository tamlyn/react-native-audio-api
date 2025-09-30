export const constants = {
  baseUrl: 'https://docs.swmansion.com/react-native-audio-api',
} as const;

export function makeDocLink(path: string) {
  return `${constants.baseUrl}${path}`;
}

export function availabilityWarn(
  feature: string,
  platform: 'web' | 'native' = 'web',
  docPath?: string
) {
  const baseMsg = `The ${feature} is not available on ${platform} platform.`;

  if (!docPath) {
    console.warn(baseMsg);
    return;
  }

  console.warn(`${baseMsg} See ${makeDocLink(docPath)} for more information.`);
}
