const { getDefaultConfig } = require('react-native/metro-config');

const config = getDefaultConfig(__dirname);

// Add web platform extensions
config.resolver.platforms = [...config.resolver.platforms, 'web'];
config.resolver.resolverMainFields = ['react-native', 'browser', 'main'];

module.exports = config;
