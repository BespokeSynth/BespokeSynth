#include "UserData.h"
#include "OpenFrameworksPort.h"

string GetBundledUserDataPath()
{
   string bundledUserDataDir;
#if JUCE_WINDOWS
   string localDataDir = File::getCurrentWorkingDirectory().getChildFile("data").getFullPathName().toStdString();
   if (juce::File(localDataDir).exists())
      bundledUserDataDir = localDataDir;
   else
      bundledUserDataDir = File::getCurrentWorkingDirectory().getChildFile("../MacOSX/build/Release/data").getFullPathName().toStdString();   //fall back to looking at OSX dir in dev environment
   ofStringReplace(bundledUserDataDir, "\\", "/");
   
#elif JUCE_LINUX
   string localDataDir = File::getCurrentWorkingDirectory().getChildFile("data").getFullPathName().toStdString();
   string developmentDataDir = File::getCurrentWorkingDirectory().getChildFile("../../MacOSX/build/Release/data").getFullPathName().toStdString();   //OSX dir in dev environment
   string installedDataDir = File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("share/BespokeSynth/data").getFullPathName().toStdString(); // /usr/share/BespokeSynth/data
   if (juce::File(localDataDir).exists())
      bundledUserDataDir = localDataDir;
   else if (juce::File(developmentDataDir).exists())
      bundledUserDataDir = developmentDataDir;
   else if (juce::File(installedDataDir).exists())
      bundledUserDataDir = installedDataDir;
   
#else
   #if DEBUG
      string relative = "../Release/data";
   #else
      string relative = "data";
   #endif
   
   string localDataDir = File::getCurrentWorkingDirectory().getChildFile(relative).getFullPathName().toStdString();
   if (juce::File(localDataDir).exists())
      bundledUserDataDir = localDataDir;
   else
      bundledUserDataDir = "/Applications/BespokeSynth/data";
#endif
   
   return bundledUserDataDir;
}

void UpdateUserData(string destDirPath)
{
   juce::File bundledDataDir(GetBundledUserDataPath());
   juce::File destDataDir(destDirPath);
   juce::File bundledDataVersionFile(bundledDataDir.getChildFile("userdata_version.txt"));
   juce::File destDataVersionFile(destDataDir.getChildFile("userdata_version.txt"));
   
   if (!bundledDataVersionFile.exists())
      return;

   bool needCopy = false;
   
   if(!destDataVersionFile.exists())
   {
      needCopy = true;
   }
   else
   {
      StringArray bundledVersionLines;
      StringArray destVersionLines;
      bundledDataVersionFile.readLines(bundledVersionLines);
      destDataVersionFile.readLines(destVersionLines);
      if (bundledVersionLines.isEmpty())
         return;
      if (destVersionLines.isEmpty())
      {
         needCopy = true;
      }
      else
      {
         int bundledDataVersion = ofToInt(bundledVersionLines[0].toStdString());
         int destDataVersion = ofToInt(destVersionLines[0].toStdString());
         needCopy = destDataVersion < bundledDataVersion;
         //I can use the data version in the future to see if special accomodations need to be made for copy/overwriting any specific files
      }
   }
   
   if (needCopy)
   {
      ofLog() << "copying data from "+bundledDataDir.getFullPathName().toStdString()+" to "+destDirPath;
      destDataDir.createDirectory();
      
      DirectoryIterator directories(bundledDataDir, true, "*", File::findDirectories);
      while (directories.next())
      {
         juce::String destDirName = juce::String(destDirPath) + "/" + directories.getFile().getRelativePathFrom(bundledDataDir);
         juce::File(destDirName).createDirectory();
      }

      DirectoryIterator entry(bundledDataDir, true);
      while (entry.next())
      {
         juce::String sourceFileName = entry.getFile().getFullPathName();
         juce::String destFileName = juce::String(destDirPath) + "/" + entry.getFile().getRelativePathFrom(bundledDataDir);

         bool copyFile = false;
         if (!juce::File(destFileName).exists())
            copyFile = true;
         if (juce::File(destFileName).exists() && !juce::File(destFileName).hasIdenticalContentTo(juce::File(sourceFileName)))
         {
            juce::File(destFileName).copyFileTo(destFileName+"_old");
            copyFile = true;
         }
             
         if (copyFile)
            juce::File(sourceFileName).copyFileTo(destFileName);
      }
   }
}
