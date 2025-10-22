# This is a set of scripts for interacting with Bespoke's cmake build system.

# Default value for CMAKE_BUILD_TYPE
default_build_type := "Debug"
# Tell cmake to use parallel execution of <number of cpu cores> minus this number
parallel_build_ignore_cpus := "2"

# Compile (default)
[positional-arguments]
build type=default_build_type:
   #!/bin/sh

   # Check if we haven't configured yet
   if ! [ -d "./ignore/build" ]; then
      # run configure step
      just _echo_do just configure "$1"
   fi

   # build with parallelism based on the number of cpu cores
   just _echo_do cmake --build ignore/build --parallel="$(nproc --ignore='{{parallel_build_ignore_cpus}}')"


# Configure build system (Release or Debug, default is Debug)
[positional-arguments]
configure type=default_build_type *cmake_args:
   #!/bin/sh

   # Find the vst2 sdk in ./ignore/VST_SDK/VST2_SDK or wherever the VST2_SDK_HOME variable points
   VST2_SDK_HOME="${VST2_SDK_HOME:-"$(pwd)"/ignore/VST_SDK/VST2_SDK}"
   if [ -d "$VST2_SDK_HOME" ]; then
      set -- "$1" -DBESPOKE_VST2_SDK_LOCATION="$VST2_SDK_HOME" "${@:2}"
   fi

   just _echo_do cmake -B ignore/build -GNinja -DCMAKE_BUILD_TYPE="$1" "${@:2}"


# Run
[positional-arguments]
run type=default_build_type *bespoke_args:
   #!/bin/sh

   # Check if we haven't compiled yet
   bespoke_exe=./ignore/build/Source/BespokeSynth_artefacts/"$1"/BespokeSynth
   if ! [ -f "$bespoke_exe" ]; then
      just _echo_do just bulid
   fi

   # Run the program
   shift
   just _echo_do "$bespoke_exe" "$@"


# Clean the build files
clean:
   rm -r ignore/build


##
## Utilities
##

# List available actions
list:
   @just --list --unsorted
   @echo
   @echo "Place your VST2 SDK at ./ignore/VST_SDK/VST2_SDK or set the VST2_SDK_HOME environment variable to enable using the VST2 SDK"


# Simple utility to print out what's being executed before running it, matching just's default format
[positional-arguments]
_echo_do *exec:
   #!/bin/bash
   echo -en '\e[1m' >&2 # style: bold
   echo -n "$@"     >&2
   echo -e '\e[0m'  >&2 # style: reset
   "$@"
