#include "FileUtils.h"
#include "OpenFrameworksPort.h"

juce::String getDirectoryOrDefault(std::string currentPath, std::string defaultPath)
{
   if (currentPath != "") {
      return juce::File(ofToDataPath(currentPath)).getParentDirectory().getFullPathName();
   } else {
      return ofToDataPath(defaultPath);
   }
}

// TODO: factor stateless methods like FileTimeComparator and ofToDataPath out of ModularSynth.cpp to here for unit testability