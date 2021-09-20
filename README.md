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

In mid-September 2021 Bespoke moved to CMake rather than Projucer for builds. The prescription to build 
Bespoke on all platforms is

```shell
git clone https://github.com/awwbees/BespokeSynth   # replace this with your fork if you forked
cd BespokeSynth
git submodule update --init --recursive
cmake -Bignore/build
cmake --build ignore/build --parallel 4
```

This will produce a release build in `ignore/build/BespokeSynth_artefacts`.

On Windows, the above cmake commands make Visual Studio project files in `ignore/build` which you can open and use directly
rather than the last step. Some windows systems also require the `-A x64` argument to force 64 bit.
On macOS if you would like to use Xcode, replace the second command with `cmake -Bignore/build -Gxcode` 
and the ignore/build directory will contain xcode project files which you can use. Finally, the CMake file has a set
of options to enable other build features which are documented in the file.
