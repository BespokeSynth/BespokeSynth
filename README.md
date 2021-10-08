[![Build Status](https://dev.azure.com/awwbees/BespokeSynth/_apis/build/status/BespokeSynth.BespokeSynth?branchName=main)](https://dev.azure.com/awwbees/BespokeSynth/_build/latest?definitionId=1&branchName=main)

# Bespoke Synth
A software modular synth that I've been building for myself since 2011, and now you can use it!

You can find the most recent builds for Mac/Windows/Linux at http://bespokesynth.com, or in the [Releases](https://github.com/awwbees/BespokeSynth/releases) section on GitHub.

Join the [Bespoke Discord](https://discord.gg/YdTMkvvpZZ) for support and to discuss with the community.

<a href='https://ko-fi.com/awwbees' target='_blank'><img height='35' style='border:0px;height:46px;' src='https://az743702.vo.msecnd.net/cdn/kofi3.png?v=0' border='0' alt='Buy Me a Coffee at ko-fi.com' />

## Screenshot
![screenshot](screenshot-1.png)

## Basic Overview/Tutorial Video
[![Bespoke Overview](https://img.youtube.com/vi/SYBc8X2IxqM/0.jpg)](https://www.youtube.com/watch?v=SYBc8X2IxqM)
* https://youtu.be/SYBc8X2IxqM

### Quick Reference
![quick reference](bespoke_quick_reference.png)

### Features
* live-patchable environment, so you can build while the music is playing
* VST hosting
* Python livecoding
* MIDI controller mapping
* Works on Windows, Mac, and Linux

### Releases
Sign up here to receive an email whenever I put out a new release: http://bespokesynth.substack.com/

### Building

Building Bespoke from source is easy and fun! The basic cmake prescription gives you a completed
executable which is ready to run on your system in many cases.

```shell
git clone https://github.com/BespokeSynth/BespokeSynth   # replace this with your fork if you forked
cd BespokeSynth
git submodule update --init --recursive
cmake -Bignore/build
cmake --build ignore/build --parallel 4
```

This will produce a release build in `ignore/build/Source/BespokeSynth_artefacts`.

There are a few useful options to the *first* cmake command which many folks choose to use.

* `-DBESPOKE_VST2_SDK_LOCATION=/path/to/sdk` will activate VST2 hosting support in your built 
copy of Bespoke if you have access to the VST SDK
* `-DBESPOKE_ASIO_SDK_LOCATION=/path/to/sdk` (windows only) will activate ASIO support on windows in your built copy of Bespoke if you have access to the ASIO SDK
* `-DBESPOKE_PYTHON_ROOT=/...` will override the automatically detected python root. In some cases with M1 mac builds in homebrew this is useful.
* `-DCMAKE_BUILD_TYPE=Debug` will produce a build with debug information available
* `-A x64` (windows only) will force visual studio to build for 64 bit architectures, in the event this is not your default
* `-GXcode` (mac only) will eject xcode project files rather than the default make files
* `-DCMAKE_INSTALL_PREFIX=/usr` (only used on Linux) will set the `CMAKE_INSTALL_PREFIX` which guides both where your
built bespoke looks for resources and also where it installs. After a build on Linux with this configured, you can
do `sudo cmake install ignore/build` and bespoke will install correctly into this directory. The cmake default is `/usr/local`.

The directory name `ignore/build` is arbitrary. Bespoke is set up to `.gitignore` everything in the `ignore` directory but you
can use any directory name you want for a build or have multiple builds also.

