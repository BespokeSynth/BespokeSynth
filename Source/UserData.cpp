#include "UserData.h"
#include "OpenFrameworksPort.h"

void UpdateUserData(string destDirPath)
{
   juce::File bundledDataDir(ofToResourcePath("userdata_original"));
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
      juce::StringArray bundledVersionLines;
      juce::StringArray destVersionLines;
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
      
      juce::DirectoryIterator directories(bundledDataDir, true, "*", juce::File::findDirectories);
      while (directories.next())
      {
         juce::String destDirName = juce::String(destDirPath) + "/" + directories.getFile().getRelativePathFrom(bundledDataDir);
         juce::File(destDirName).createDirectory();
      }

      juce::DirectoryIterator entry(bundledDataDir, true);
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
