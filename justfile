# This is a set of scripts for interacting with Bespoke's cmake build system.

# Default values for environment variables
export BESPOKE_BUILD_TYPE := env("BESPOKE_BUILD_TYPE", "Debug")

# Simple utility to print out what's being executed before running it, matching just's default format
echo_do := "echo_do() {
      [ -p /dev/stdout ] || \\
      echo -en '\\e[1m' >&2 # style: bold
      echo -n \"$@\"    >&2
      [ -p /dev/stdout ] || \\
      echo -e '\\e[0m'  >&2 # style: reset
      \"$@\"
   }"


# Aliases for common commands
alias br := build-run
alias rel := release
alias b := build
alias r := run
alias l := list


# Equivalent to `just build run` (default)
[positional-arguments]
build-run *command_prefix:
   #!/usr/bin/env sh
   {{echo_do}}

   echo_do just build run "$@"


# Run subsequent `just` commands in release mode
[no-cd]
[positional-arguments]
release *just_commands:
   #!/usr/bin/env sh
   {{echo_do}}

   echo_do export BESPOKE_BUILD_TYPE=Release
   echo_do just "$@"


# Compile
[positional-arguments]
build:
   #!/usr/bin/env sh
   {{echo_do}}

   # Check if we haven't configured yet
   if ! [ -d "./ignore/build/$BESPOKE_BUILD_TYPE" ]; then
      # configure
      echo_do just configure
   fi

   # build with parallelism based on the number of cpu cores
   echo_do cmake --build ignore/build/"$BESPOKE_BUILD_TYPE" --parallel="${PARALLEL_BUILD_CPUS:-"$(nproc)"}"


# Configure the build system (Release or Debug, default is Debug)
[positional-arguments]
configure *cmake_args:
   #!/usr/bin/env sh
   {{echo_do}}

   # Find the vst2 sdk in ./ignore/VST_SDK/VST2_SDK or wherever the VST2_SDK_HOME variable points
   VST2_SDK_HOME="${VST2_SDK_HOME:-"$(pwd)"/ignore/VST_SDK/VST2_SDK}"
   if [ -d "$VST2_SDK_HOME" ]; then
      set -- -DBESPOKE_VST2_SDK_LOCATION="$VST2_SDK_HOME" "$@"
   fi

   echo_do cmake -B ignore/build/"$BESPOKE_BUILD_TYPE" -GNinja -DCMAKE_BUILD_TYPE="$BESPOKE_BUILD_TYPE" "$@"


# Run (with an optional command prefix, such as `gdb`)
[positional-arguments]
run *command_prefix:
   #!/usr/bin/env sh
   {{echo_do}}

   # Check if we haven't compiled yet
   bespoke_exe="$(just _print_binary_name)"
   if ! [ -f "$bespoke_exe" ]; then
      echo_do just build
   fi

   # Run the program
   echo_do "$@" "$bespoke_exe"


# Clean the build files
clean:
   rm -r ignore/build


##
## Utilities
##


_print_binary_name:
   #!/usr/bin/env sh
   bespoke_exe=./ignore/build/"$BESPOKE_BUILD_TYPE"/Source/BespokeSynth_artefacts/"$BESPOKE_BUILD_TYPE"/BespokeSynth
   realpath "$bespoke_exe"


# List available actions
list:
   @just --list --unsorted
   @echo
   @echo "Place your VST2 SDK at ./ignore/VST_SDK/VST2_SDK or set the VST2_SDK_HOME environment variable to enable using the VST2 SDK"


# Allow extending this file locally (customizing recipes, setting environment variables, etc.)
import? 'ignore/local.just'