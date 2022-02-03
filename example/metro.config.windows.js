/**
 * Metro configuration for React Native
 * https://github.com/facebook/react-native
 *
 * @format
 */
const path = require('path');
const exclusionList = require('metro-config/src/defaults/exclusionList');

const rnwPath = path.resolve(
  require.resolve('react-native-windows/package.json'),
  '..',
);
const libPath = path.resolve(__dirname, '..');

module.exports = {
  projectRoot: __dirname,
  watchFolders: [libPath],
  resolver: {
    // Metro can't handle symlinks, so provide the real path to find our lib.
    // For all other modules, just look in node_modules
    extraNodeModules: new Proxy(
      {
        'react-native-orientation-locker': libPath,
      },
      {
        get: (target, name) =>
          target[name] ?? path.resolve(__dirname, 'node_modules', name),
      },
    ),
    // Include the macos platform in addition to the defaults because the fork includes macos, but doesn't declare it
    platforms: ['ios', 'android', 'windesktop', 'windows', 'web', 'macos'],
    blockList: exclusionList([
      // This stops "react-native run-windows" from causing the metro server to crash if its already running
      new RegExp(
        `${path.resolve(__dirname, 'windows').replace(/[/\\]/g, '/')}.*`,
      ),
      // This prevents "react-native run-windows" from hitting: EBUSY: resource busy or locked, open msbuild.ProjectImports.zip or other files produced by msbuild
      new RegExp(`${rnwPath}/build/.*`),
      new RegExp(`${rnwPath}/target/.*`),
      /.*\.ProjectImports\.zip/,
    ]),
  },
  transformer: {
    getTransformOptions: async () => ({
      transform: {
        experimentalImportSupport: false,
        inlineRequires: true,
      },
    }),
  },
};
