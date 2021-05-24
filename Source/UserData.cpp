#include "UserData.h"

bool updateUserData()
{
    juce::File sDataDir (File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("share/BespokeSynth").getFullPathName());
    juce::File localDataDir (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("BespokeSynth").getFullPathName());
    #if BESPOKE_MAC
    juce::File sConfig (File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("share/BespokeSynth/data/userprefs.json").getFullPathName());
    #elif BESPOKE_LINUX
    juce::File sConfig (File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("share/BespokeSynth/data/userprefs_linux.json").getFullPathName());
    #else
    juce::File sConfig (File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("share/BespokeSynth/data/userprefs_win.json").getFullPathName());
    #endif
    juce::File localConfig (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("BespokeSynth/data/userprefs.json").getFullPathName());
    juce::File localBlank (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("BespokeSynth/data/layouts/blank.json").getFullPathName());
    
    juce::File localConfigSave (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("BespokeSynth/data/userprefs_save.json").getFullPathName());
    juce::File localBlankSave (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("BespokeSynth/data/layouts/blank_save.json").getFullPathName());
    
    if(juce::File(localDataDir).exists())
    {
        localConfig.copyFileTo(localConfigSave);
        localBlank.copyFileTo(localBlankSave);
        
        DBG("Updating sData to uData");
        
        juce::String sFileName, shortFileName, uFileName;
        juce::String uDataDir = localDataDir.getFullPathName();
        
        for (DirectoryEntry entry : RangedDirectoryIterator (File (sDataDir), true))
        {
            sFileName = entry.getFile().getFullPathName();
            
            uFileName = uDataDir + "/" + entry.getFile().getRelativePathFrom(sDataDir);
            
            if (!(juce::File(uFileName).exists()) || !juce::File(sFileName).hasIdenticalContentTo(juce::File(uFileName)))
                juce::File(sFileName).copyFileTo(uFileName);
        }
        
        localConfigSave.replaceFileIn(localConfig);
        localBlankSave.replaceFileIn(localBlank);
        return true;
    }
    else
    {
        DBG("Copying sData to uData");
//         sDataDir.copyDirectoryTo(localDataDir);
//         sConfig.copyFileTo(localConfig);
        return (sDataDir.copyDirectoryTo(localDataDir) && sConfig.copyFileTo(localConfig));
    }
    
    return false;
}
