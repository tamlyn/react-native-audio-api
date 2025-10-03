import makeDocLink from './makeDocLink';

export default function availabilityWarn(
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
