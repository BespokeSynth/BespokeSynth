#pragma once

#include <juce_core/juce_core.h>
#include <string>

juce::String getDirectoryOrDefault(std::string currentPath, std::string defaultPath);
