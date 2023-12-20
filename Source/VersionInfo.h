#include <string>

namespace Bespoke
{
   // These are generated at CMake time and are VersionInfo.cpp.in
   extern const char* APP_NAME; // The application name. Taken from the top-level CMake project name.
   extern const char* VERSION; // This will be the same string as Juce::Application::getApplicationVersion()
   extern const char* PYTHON_VERSION; // The python version we compiled with
   extern const char* CMAKE_INSTALL_PREFIX;

   // These are generated at build time and are in VersionInfoBld.cpp.in
   extern const char* GIT_BRANCH;
   extern const char* GIT_HASH;
   extern const char* BUILD_ARCH;
   extern const char* BUILD_DATE;
   extern const char* BUILD_TIME;
}
