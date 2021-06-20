/*
  ==============================================================================

    SampleBrowser.cpp
    Created: 19 Jun 2021 6:46:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SampleBrowser.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

SampleBrowser::SampleBrowser()
{
   mCurrentDirectory = ofToDataPath("samples");
}

SampleBrowser::~SampleBrowser()
{
}

void SampleBrowser::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK(3, 20);
   for (int i=0; i<(int)mButtons.size(); ++i)
   {
      BUTTON(mButtons[i], ("button"+ofToString(i)).c_str());
   }
   BUTTON(mBackButton, " < ");
   UIBLOCK_SHIFTX(80);
   BUTTON(mForwardButton, " > ");
   ENDUIBLOCK0();
   
   SetDirectory(mCurrentDirectory);
}

void SampleBrowser::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   float fontSize = 15;
   float stringWidth = gFont.GetStringWidth(mCurrentDirectory.toStdString(),fontSize,K(isRenderThread));
   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   float textX = 3;
   if (stringWidth > moduleWidth)
      textX = moduleWidth - 3 - stringWidth;
   gFont.DrawString(mCurrentDirectory.toStdString(), fontSize, textX, 15);
   
   for (size_t i=0; i<mButtons.size(); ++i)
      mButtons[i]->Draw();
   mBackButton->Draw();
   mForwardButton->Draw();
   
   int numPages = GetNumPages();
   if (numPages > 1)
      DrawTextNormal(ofToString(mCurrentPage+1)+"/"+ofToString(numPages), 40, mBackButton->GetPosition(true).y+12);
}

void SampleBrowser::ButtonClicked(ClickButton* button)
{
   if (button == mBackButton)
      ShowPage(mCurrentPage-1);
   if (button == mForwardButton)
      ShowPage(mCurrentPage+1);
   for (int i=0; i<(int)mButtons.size(); ++i)
   {
      if (button == mButtons[i])
      {
         int offset = mCurrentPage * (int)mButtons.size();
         int entryIndex = offset + i;
         if (entryIndex < (int)mDirectoryListing.size())
         {
            String clicked = mDirectoryListing[entryIndex];
            if (clicked == "..")
               SetDirectory(File(mCurrentDirectory).getParentDirectory().getFullPathName());
            else if (!clicked.contains("."))
               SetDirectory(File(mCurrentDirectory).getChildFile(clicked).getFullPathName());
            else
               TheSynth->GrabSample(File(mCurrentDirectory).getChildFile(clicked).getFullPathName().toStdString());
         }
      }
   }
}

namespace
{
   int CompareDirectoryListing(const String& text, const String& other)
   {
      if (text == other)
         return 0;
      bool isDir = text == ".." || !text.contains(".");
      bool isOtherDir = other == ".." || !other.contains(".");
      if (isDir && !isOtherDir)
         return -1;
      if (!isDir && isOtherDir)
         return 1;
      return text.compareIgnoreCase(other);
   }

   void SortDirectoryListing(StringArray& listing)
   {
      std::sort(listing.begin(), listing.end(), [](const String& a, const String& b) { return CompareDirectoryListing(a, b) < 0; });
   }
}

void SampleBrowser::SetDirectory(String dirPath)
{
   mCurrentDirectory = dirPath;
   
   mDirectoryListing.clear();
   
   String matcher = TheSynth->GetGlobalManagers()->mAudioFormatManager.getWildcardForAllFormats();
   
   StringArray wildcards;
   wildcards.addTokens(matcher, ";,", "\"'");
   wildcards.trim();
   wildcards.removeEmptyStrings();
   
   File dir(ofToDataPath(dirPath.toStdString()));
   if (dir.getParentDirectory().getFullPathName() != dir.getFullPathName())
      mDirectoryListing.add("..");
   for (auto file : dir.findChildFiles(File::findFilesAndDirectories|File::ignoreHiddenFiles, false))
   {
      bool include = false;
      if (file.isDirectory())
      {
         include = true;
      }
      else
      {
         for (auto& w : wildcards)
         {
             if (file.getFileName().matchesWildcard (w, !File::areFileNamesCaseSensitive()))
             {
                include = true;
                break;
             }
         }
      }
      if (include)
         mDirectoryListing.add(file.getFileName());
   }
   SortDirectoryListing(mDirectoryListing);
            
   ShowPage(0);
}

void SampleBrowser::ShowPage(int page)
{
   page = ofClamp(page, 0, GetNumPages()-1);
   mCurrentPage = page;
   int offset = page * (int)mButtons.size();
   for (int i=0; i<(int)mButtons.size(); ++i)
   {
      if (i+offset < (int)mDirectoryListing.size())
      {
         mButtons[i]->SetShowing(true);
         if (mDirectoryListing[i + offset] == ".." || !mDirectoryListing[i + offset].contains("."))
            mButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kFolderIcon);
         else
            mButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kSampleIcon);
         mButtons[i]->SetLabel(mDirectoryListing[i+offset].toStdString().c_str());
      }
      else
      {
         mButtons[i]->SetShowing(false);
      }
   }
   
   mBackButton->SetShowing(GetNumPages() > 1 && mCurrentPage > 0);
   mForwardButton->SetShowing(GetNumPages() > 1 && mCurrentPage < GetNumPages() - 1);
}

int SampleBrowser::GetNumPages() const
{
   return MAX((int)ceil((float)mDirectoryListing.size() / mButtons.size()), 1);
}

void SampleBrowser::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void SampleBrowser::SetUpFromSaveData()
{
}

namespace
{
   const int kSaveStateRev = 0;
}

void SampleBrowser::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mCurrentDirectory.toStdString();
}

void SampleBrowser::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   string currentDirectory;
   in >> currentDirectory;
   SetDirectory(currentDirectory);
}

