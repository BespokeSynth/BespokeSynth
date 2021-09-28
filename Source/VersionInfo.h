#include <string>

namespace Bespoke
{
    extern const char* APP_NAME; // The application name. Taken from the top-level CMake project name.
    extern const char* VERSION; // This will be the same string as Juce::Application::getApplicationVersion()
    extern const char* PYTHON_VERSION; // The python version we compiled with
    extern const char* CMAKE_INSTALL_PREFIX;
}