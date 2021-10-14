cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)

# Global build parameters. Optional features are configurable via additional
# BESPOKE_* variables defined in their respective subdirs.
set(BESPOKE_JUCE_LOCATION "${CMAKE_SOURCE_DIR}/libs/JUCE" CACHE STRING "Path to JUCE library source tree")
set(BESPOKE_PYTHON_ROOT "" CACHE STRING "Python search path. Override as needed.")
set(BESPOKE_VST2_SDK_LOCATION "" CACHE STRING "Steinberg VST2 SDK directory for non-FOSS builds")
set(BESPOKE_ASIO_SDK_LOCATION "" CACHE STRING "Steinberg ASIO SDK directory for non-FOSS builds")
set(BESPOKE_LINUX_HIGH_FPS_WITH_THREAD_PROBLEMS 0 CACHE BOOL "Enable if you want 60fps on Linux, but there's the possibility that you will get thread safety crashes. This is being investigated for a future release")

# Global CMake options
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT BespokeSynth)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS version")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project(BespokeSynth VERSION 1.0.1 LANGUAGES C CXX ASM)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Global compile options
add_compile_options(
    # Build with Multiple Processes on Visual Studio
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
    # Set source and executable charsets to UTF-8. Required for building on CJK Windows.
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )

if(BESPOKE_PYTHON_ROOT)
    set(Python_ROOT "${BESPOKE_PYTHON_ROOT}")
    message(STATUS "Overriding Python_ROOT to ${Python_ROOT}")
endif()
find_package (Python COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python version: ${Python_VERSION}")
message(STATUS "Python executable: ${Python_EXECUTABLE}")
message(STATUS "Python library: ${Python_LIBRARIES}")

message(STATUS "Using JUCE from ${BESPOKE_JUCE_LOCATION}")
add_subdirectory(${BESPOKE_JUCE_LOCATION} ${CMAKE_BINARY_DIR}/JUCE EXCLUDE_FROM_ALL)

if (BESPOKE_VST2_SDK_LOCATION)
    juce_set_vst2_sdk_path(${BESPOKE_VST2_SDK_LOCATION})
    message(STATUS "Enabling VST2 - caution, non FOSS build")
endif()

add_subdirectory(libs)
add_subdirectory(BespokeVSTScanner)
add_subdirectory(Source)
