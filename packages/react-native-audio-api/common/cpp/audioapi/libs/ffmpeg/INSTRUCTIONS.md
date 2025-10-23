# Instruction on how to compile your own version of the ffmpeg and to replace binary files with yours.

If you would like to change ffmpeg binaries f.e. to include some of its functionalities. There are possible solutions to do that.

Basically, there are two options to achieve it:

- Utilize `ffmpeg_setup.sh` script, which provides set of instructions to cross-compile FFmpeg library on all desired mobile architectures.
- Cross-compile it dynamically fully on your own.

Last part in both of the cases is to replace files in corresponding places.

## Where to put compiled files?

### Includes

All includes should go to packages/react-native-audio-api/common/cpp/audioapi/external/ffmpeg_include

### Android

Put your corresponding .so files for each architecture (TARGET_ABI) to packages/react-native-audio-api/android/src/main/jniLibs/TARGET_ABI

### iOS

Put corresponding .xcframework files in packages/react-native-audio-api/common/cpp/audioapi/external

## Additional steps - IMPORTANT!

- after moving the corresponding platform files, delete the all output files from this directory
  - otherwise XCode will try to link them
- remove `time.h` from include directories as it will conflict with system one in XCode
- rename all local occurrences of `AVMediaType` to f.e. `AVMediaTypeFFMPEG`
- finally remove `build` directory from your example app `ios` directory as most likely XCode will choke on that too
