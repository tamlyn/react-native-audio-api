const { getDefaultConfig } = require('expo/metro-config');
const {
  wrapWithAudioAPIMetroConfig,
} = require('react-native-audio-api/metro-config');
const path = require('path');

const projectRoot = __dirname;
const monorepoRoot = path.resolve(projectRoot, '../..');

const config = getDefaultConfig(projectRoot);

config.watchFolders = [monorepoRoot];

// Important: Tell Metro where to find node_modules - prioritize local first
config.resolver.nodeModulesPaths = [
  path.resolve(projectRoot, 'node_modules'),
  path.resolve(monorepoRoot, 'node_modules'),
];

// Ensure React and React Native are resolved from local node_modules only
config.resolver.resolveRequest = (context, moduleName, platform) => {
  if (moduleName === 'react' || moduleName === 'react-native' || moduleName.startsWith('react-native/')) {
    const localNodeModules = path.resolve(projectRoot, 'node_modules');
    return context.resolveRequest(
      { ...context, nodeModulesPaths: [localNodeModules] },
      moduleName,
      platform
    );
  }
  return context.resolveRequest(context, moduleName, platform);
};

module.exports = wrapWithAudioAPIMetroConfig(config);
