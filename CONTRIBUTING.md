<!-- omit in toc -->
# Contributing to Bespoke Synth

First off all, thank you for taking the time to contribute! ❤️

All types of contributions are encouraged and valued. See the [Table of Contents](#table-of-contents) for different ways to help and details about how this project handles them. Please make sure to read the relevant section before making your contribution. It will make it a lot easier for us maintainers and smooth out the experience for all involved. The community looks forward to your contributions. 🎉

> And if you like the project, but just don't have time to contribute, that's fine. There are other easy ways to support the project and show your appreciation, which we would also be very happy about:
> - Star the project
> - Express about it on your favorite social media outlet
> - Refer this project in your project's readme
> - Mention the project at local meetups and tell your friends/colleagues
> - Share the [Bespoke Discord](https://discord.gg/YdTMkvvpZZ)


<!-- omit in toc -->
## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [I Have a Question](#i-have-a-question)
  - [I Want To Contribute](#i-want-to-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Enhancements](#suggesting-enhancements)
  - [Code Contribution](#code-contributing)
    - [Setting up your environment](#setting-up-your-environment)
    - [Building](#building)
    - [Submitting your code](#submitting-your-code)
  - [Improving The Documentation](#improving-the-documentation)
- [Style guides](#style-guides)
  - [Commit Messages](#commit-messages)
- [Join The Project Team](#join-the-project-team)


## Code of Conduct

This project and everyone participating in it is governed by the [Bespoke Synth Code of Conduct](https://github.com/BespokeSynth/BespokeSynth/blob//CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code. Please report unacceptable behavior on [our discord](https://discord.gg/YdTMkvvpZZ) to the moderators or to <bespokesynth@gmail.com>.


## I Have a Question

> If you want to ask a question, we assume that you have read the available [Documentation](https://www.bespokesynth.com/docs/index.html) or the [Community-written documentation](https://github.com/BespokeSynth/BespokeSynthDocs/wiki).

Before you ask a question, it is best to search for existing [Issues](https://github.com/BespokeSynth/BespokeSynth/issues) that might help you and/or ask on [our Discord](https://discord.gg/YdTMkvvpZZ). In case you have found a suitable issue and still need clarification, you can write your question in this issue. It is also advisable to search the internet for answers first.

If you then still feel the need to ask a question and need clarification, we recommend the following:

- Open an [Issue](https://github.com/BespokeSynth/BespokeSynth/issues/new).
- Provide as much context as you can about what you're running into.
- Provide project, platform and plugin versions.
- Provide a savestate to reproduce the issue, when applicable.

We will then take care of the issue as soon as possible.


## I Want To Contribute

> ### Legal Notice <!-- omit in toc -->
> When contributing to this project, you must agree that you have authored 100% of the content, that you have the necessary rights to the content and that the content you contribute may be provided under the project license.


### Reporting Bugs


<!-- omit in toc -->
#### Before Submitting a Bug Report

We ask you to investigate carefully, collect information and describe the issue in detail in your report. Please complete the following steps in advance to help us fix any potential bug as fast as possible.

- Make sure that you are using the latest version.
- Determine if your bug is really a bug and not an error on your side e.g. using incompatible environment components/versions (Make sure that you have read the [documentation](https://www.bespokesynth.com/docs/index.html) and the [community-written documentation](https://github.com/BespokeSynth/BespokeSynthDocs/wiki). If you are looking for support, you might want to check [this section](#i-have-a-question)).
- To see if other users have experienced (and potentially already solved) the same issue you are having, check if there is not already a bug report existing for your bug or error in the [bug tracker](https://github.com/BespokeSynth/BespokeSynth/issues?q=label%3Abug).
- Also make sure to search [our Discord](https://discord.gg/YdTMkvvpZZ) to see if other users have discussed the issue.
- Collect information about the bug:
  - Stack trace (Traceback)
  - OS, Platform and Version (Windows, Linux, macOS, x86, ARM)
  - Version of plugins used if relevant.
  - System information and settings like Audio hardware and software, samplerate, buffer size and related.
  - Can you reliably reproduce the issue? And can you also reproduce it with older versions?


<!-- omit in toc -->
#### How Do I Submit a Good Bug Report?

> You must never report security related issues, vulnerabilities or bugs including sensitive information to the issue tracker, or elsewhere in public. Instead sensitive bugs must be sent by email to <bespokesynth@gmail.com>.

We use GitHub issues to track bugs and errors. If you run into an issue with the project:

- Open an [Issue](https://github.com/BespokeSynth/BespokeSynth/issues/new). (Since we can't be sure at this point whether it is a bug or not, we ask you not to talk about a bug yet and not to label the issue.)
- Explain the behavior you would expect and the actual behavior.
- Please provide as much context as possible and describe the *reproduction steps* that someone else can follow to recreate the issue on their own. This usually includes your code. For good bug reports you should isolate the problem and create a reduced test case.
- Provide the information you collected in the previous section.

Once it's filed:

- The project team will label the issue accordingly.
- A team member will try to reproduce the issue with your provided steps. If there are no reproduction steps or no obvious way to reproduce the issue, the team will ask you for those steps and mark the issue as `need more info`. Bugs with the `need more info` tag will not be addressed until they are reproduced.
- If the team is able to reproduce the issue, they likely will mention this in the issue, as well as possibly add other tags (such as the size or priority), and the issue will be left to be [resolved by a code contributor](#code-contributing).


### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for Bespoke Synth, **including completely new features and minor improvements to existing functionality**. Following these guidelines will help maintainers and the community to understand your suggestion and find related suggestions.


<!-- omit in toc -->
#### Before Submitting an Enhancement

- Make sure that you are using the latest version.
- Read the [documentation](https://www.bespokesynth.com/docs/index.html) and the [community-written documentation](https://github.com/BespokeSynth/BespokeSynthDocs/wiki) carefully and find out if the functionality is already covered, maybe by an individual configuration.
- Perform a [search](https://github.com/BespokeSynth/BespokeSynth/issues) to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.
- Check on [our Discord](https://discord.gg/YdTMkvvpZZ) to see if the feature already exists or to discuss it. There is the #feature-request channel to ask or discuss enhancements.
- Find out whether your idea fits with the scope and aims of the project. It's up to you to make a strong case to convince the project's developers of the merits of this feature.


<!-- omit in toc -->
#### How Do I Submit a Good Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](https://github.com/BespokeSynth/BespokeSynth/issues).

- Use a **clear and descriptive title** for the issue to identify the suggestion.
- Provide a **step-by-step description of the suggested enhancement** in as many details as possible.
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why. At this point you can also tell which alternatives do not work for you.
- You may want to **include screenshots or screen recordings** which help you demonstrate the steps or point out the part which the suggestion is related to. You can use [LICEcap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and the built-in [screen recorder in GNOME](https://help.gnome.org/users/gnome-help/stable/screen-shot-record.html.en) or [SimpleScreenRecorder](https://github.com/MaartenBaert/ssr) on Linux.
- **Explain why this enhancement would be useful** to most Bespoke Synth users. You may also want to point out the other projects that solved it better and which could serve as inspiration.


### Code Contributing


#### Setting up your environment

To be able to build you will need a few things, depending on your OS

* All systems require an install of git
* On Windows: 
  * Install Microsoft Visual Studio 2019 (or 2022) Community Edition. When you install Microsoft Visual Studio, make sure to include CLI tools and CMake, which are included in 'Optional CLI support' and 'Toolset for Desktop' install bundles
  * Python from python.org
  * Run all commands from the visual studio command shell which will be available after you install MSVS.
* On MacOS: install xcode; install xcode command line tools with `xcode-select --install` and install cmake with `brew install cmake` if you use homebrew or from cmake.org if not
* On Linux you probably already have everything (gcc, git, etc...), but you will need to install required packages. The full list we install on a fresh ubuntu 20 box are listed in the azure-pipelines.yml
  * Some distributions may have slightly different package names like for instance Debian bookworm: You need to replace `alsa` and `alsa-tools` with `alsa-utils`


#### Building

Building Bespoke from source is easy and fun! The basic cmake prescription gives you a completed executable which is ready to run on your system in many cases. If your system does not have `cmake` installed already you must do so.

```shell
git clone https://github.com/BespokeSynth/BespokeSynth   # Replace this with your fork if you forked
cd BespokeSynth
git submodule update --init --recursive
cmake -Bignore/build -DCMAKE_BUILD_TYPE=Release
cmake --build ignore/build --parallel 4 --config Release  # You can supply a larger number than 4 if you have more cpu cores.
```

This will produce a release build in `ignore/build/Source/BespokeSynth_artefacts`.

There are a few useful options to the *first* cmake command which many folks choose to use.

* `-DBESPOKE_VST2_SDK_LOCATION=/path/to/sdk` will activate VST2 hosting support in your built copy of Bespoke if you have access to the VST SDK
* `-DBESPOKE_ASIO_SDK_LOCATION=/path/to/sdk` (windows only) will activate ASIO support on windows in your built copy of Bespoke if you have access to the ASIO SDK
* `-DBESPOKE_SPACEMOUSE_SDK_LOCATION=/path/to/sdk` (windows only) will activate SpaceMouse canvas navigation support on windows in your built copy of Bespoke if you have access to the SpaceMouse SDK
* `-DBESPOKE_PYTHON_ROOT=/...` will override the automatically detected python root. In some cases with M1 mac builds in homebrew this is useful.
* `-DCMAKE_BUILD_TYPE=Debug` will produce a build with debug information available
* `-A x64` (windows only) will force visual studio to build for 64 bit architectures, in the event this is not your default
* `-GXcode` (mac only) will eject xcode project files rather than the default make files
* `-DCMAKE_INSTALL_PREFIX=/usr` (only used on Linux) will set the `CMAKE_INSTALL_PREFIX` which guides both where your built bespoke looks for resources and also where it installs. After a build on Linux with this configured, you can do `sudo cmake --install ignore/build` and bespoke will install correctly into this directory. The cmake default is `/usr/local`.

The directory name `ignore/build` is arbitrary. Bespoke is set up to `.gitignore` everything in the `ignore` directory but you can use any directory name you want for a build or have multiple builds also.


#### Submitting your code

We make use of [Pull Requests on github](https://github.com/BespokeSynth/BespokeSynth/compare) to review and merge your contribution with the `main` branch.

We use the `Squash and merge` method which means you can make as many commits to your pull request as you desire and the final merge will have the creator of the pull request as author of the singular merged commit.


### Improving The Documentation

You can directly contribute and help write the [Community-written documentation](https://github.com/BespokeSynth/BespokeSynthDocs/wiki).

For improving the main [documentation](https://www.bespokesynth.com/docs/index.html) you can [create a pull request](https://github.com/BespokeSynth/BespokeSynth/compare) to the source repository since this documentation is generated from the source. This also applies to the tooltips and other in-application documentation.


## Style guides

We use Clang format to to keep the code formatting in check. Almost all IDE's support the `.clang-format` file but some may need an extension or change in settings to make use of it.


<!-- omit in toc -->
## Attribution
This guide is based on the [contributing.md](https://contributing.md/generator)!
