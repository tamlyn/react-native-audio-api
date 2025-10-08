/** @type {import('eslint').ESLint.ConfigData} */
module.exports = {
  extends: ['../../.eslintrc.js'],
  overrides: [
    {
      files: ['./src/**/*.{ts,tsx}'],
    },
  ],
  ignorePatterns: ['lib', 'src/external/signalsmithStretch'],
  settings: {
    'import/resolver': {
      node: {
        extensions: [
          '.js',
          '.jsx',
          '.ts',
          '.tsx',
          '.d.ts',
          '.json',
          '.web.ts',
          '.web.tsx',
          '.native.ts',
          '.native.tsx',
          '.ios.ts',
          '.ios.tsx',
          '.android.ts',
          '.android.tsx',
        ],
      },
    },
  },
};
