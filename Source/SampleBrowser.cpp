/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
#include "IAudioReceiver.h"
#include "Profiler.h"

#include "juce_audio_formats/juce_audio_formats.h"

using namespace juce;

SampleBrowser::SampleBrowser()
{
   mCurrentDirectory = ofToSamplePath("");
}

SampleBrowser::~SampleBrowser()
{
}

void SampleBrowser::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 20);
   for (int i = 0; i < (int)mButtons.size(); ++i)
   {
      xOffset += 270;
      BUTTON(mPlayButtons[i], ("play" + ofToString(i)).c_str());
      mPlayButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kPlay);
      mPlayButtons[i]->SetDimensions(20, 15);
      UIBLOCK_SHIFTX(-270);
      BUTTON(mButtons[i], ("button" + ofToString(i)).c_str());
      UIBLOCK_NEWLINE();
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

   float fontSize = 13;
   float stringWidth = gFont.GetStringWidth(mCurrentDirectory.toStdString(), fontSize);
   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   float textX = 3;
   if (stringWidth > moduleWidth)
      textX = moduleWidth - 3 - stringWidth;
   gFont.DrawString(mCurrentDirectory.toStdString(), fontSize, textX, 15);

   for (size_t i = 0; i < mButtons.size(); ++i)
      mButtons[i]->Draw();
   for (int i = 0; i < (int)mPlayButtons.size(); ++i)
   {
      mPlayButtons[i]->SetDisplayStyle(IsSamplePlaying(i) ? ButtonDisplayStyle::kStop : ButtonDisplayStyle::kPlay);
      mPlayButtons[i]->Draw();
   }
   mBackButton->Draw();
   mForwardButton->Draw();

   int numPages = GetNumPages();
   if (numPages > 1)
      DrawTextNormal(ofToString(mCurrentPage + 1) + "/" + ofToString(numPages), 40, mBackButton->GetPosition(true).y + 12);
}

bool SampleBrowser::IsSamplePlaying(int index) const
{
   return mPlayingSample.IsPlaying() && mButtons[index]->IsShowing() && mDirectoryListing[index] == juce::String(mPlayingSample.GetReadPath());
}

void SampleBrowser::ButtonClicked(ClickButton* button, double time)
{
   if (button == mBackButton)
      ShowPage(mCurrentPage - 1);
   if (button == mForwardButton)
      ShowPage(mCurrentPage + 1);
   for (int i = 0; i < (int)mButtons.size(); ++i)
   {
      if (button == mButtons[i] || button == mPlayButtons[i])
      {
         int offset = mCurrentPage * (int)mButtons.size();
         int entryIndex = offset + i;
         if (entryIndex < (int)mDirectoryListing.size())
         {
            String clicked = mDirectoryListing[entryIndex];
            if (button == mButtons[i])
            {
               if (clicked == "..")
               {
                  File dir(mCurrentDirectory);
                  if (dir.getParentDirectory().getFullPathName() != dir.getFullPathName())
                     SetDirectory(File(mCurrentDirectory).getParentDirectory().getFullPathName());
                  else
                     SetDirectory("");
               }
               else if (File(clicked).isDirectory())
               {
                  SetDirectory(clicked);
               }
               else
               {
                  TheSynth->GrabSample(clicked.toStdString());
               }
            }
            if (button == mPlayButtons[i])
            {
               if (IsSamplePlaying(i))
               {
                  mPlayingSample.Reset();
               }
               else
               {
                  if (File(clicked).existsAsFile())
                  {
                     mSampleMutex.lock();
                     mPlayingSample.SetName(clicked.toStdString().c_str());
                     mPlayingSample.Read(clicked.toStdString().c_str());
                     mPlayingSample.Play(NextBufferTime(false), 1, 0);
                     mSampleMutex.unlock();
                  }
               }
            }
         }
      }
   }
}

void SampleBrowser::Process(double time)
{
   PROFILER(SampleBrowser);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   gWorkChannelBuffer.Clear();
   mSampleMutex.lock();
   if (mPlayingSample.IsPlaying())
      mPlayingSample.ConsumeData(time, &gWorkChannelBuffer, bufferSize, true);
   mSampleMutex.unlock();

   const int kNumChannels = 2;
   if (gWorkChannelBuffer.NumActiveChannels() == 1)
   {
      gWorkChannelBuffer.SetNumActiveChannels(2);
      BufferCopy(gWorkChannelBuffer.GetChannel(1), gWorkChannelBuffer.GetChannel(0), bufferSize);
   }
   SyncOutputBuffer(kNumChannels);
   for (int ch = 0; ch < kNumChannels; ++ch)
   {
      GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
   }
}

namespace
{
   int CompareDirectoryListing(const String& text, const String& other)
   {
      if (text == other)
         return 0;
      bool isDir = text == ".." || File(text).isDirectory();
      bool isOtherDir = other == ".." || File(other).isDirectory();
      if (isDir && !isOtherDir)
         return -1;
      if (!isDir && isOtherDir)
         return 1;
      return text.compareIgnoreCase(other);
   }

   void SortDirectoryListing(StringArray& listing)
   {
      std::sort(listing.begin(), listing.end(), [](const String& a, const String& b)
                {
                   return CompareDirectoryListing(a, b) < 0;
                });
   }
}

void SampleBrowser::SetDirectory(String dirPath)
{
   mCurrentDirectory = dirPath;

   mDirectoryListing.clear();

   if (dirPath != "")
   {
      String matcher = TheSynth->GetAudioFormatManager().getWildcardForAllFormats();

      StringArray wildcards;
      wildcards.addTokens(matcher, ";,", "\"'");
      wildcards.trim();
      wildcards.removeEmptyStrings();

      mDirectoryListing.add("..");

      File dir(ofToSamplePath(dirPath.toStdString()));
      for (auto file : dir.findChildFiles(File::findFilesAndDirectories | File::ignoreHiddenFiles, false))
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
               if (file.getFileName().matchesWildcard(w, true))
               {
                  include = true;
                  break;
               }
            }
         }
         if (include)
            mDirectoryListing.add(file.getFullPathName());
      }
   }
   else
   {
      Array<File> roots;
      File::findFileSystemRoots(roots);
      for (auto root : roots)
         mDirectoryListing.add(root.getFullPathName());
   }
   SortDirectoryListing(mDirectoryListing);

   ShowPage(0);
}

void SampleBrowser::ShowPage(int page)
{
   page = ofClamp(page, 0, GetNumPages() - 1);
   mCurrentPage = page;
   int offset = page * (int)mButtons.size();
   for (int i = 0; i < (int)mButtons.size(); ++i)
   {
      if (i + offset < (int)mDirectoryListing.size())
      {
         mButtons[i]->SetShowing(true);
         if (mDirectoryListing[i + offset] == ".." || File(mDirectoryListing[i + offset]).isDirectory())
         {
            mButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kFolderIcon);
            mPlayButtons[i]->SetShowing(false);
         }
         else
         {
            mButtons[i]->SetDisplayStyle(ButtonDisplayStyle::kSampleIcon);
            mPlayButtons[i]->SetShowing(true);
         }

         if (mDirectoryListing[i + offset] == "..")
            mButtons[i]->SetLabel("..");
         else
            mButtons[i]->SetLabel(File(mDirectoryListing[i + offset]).getFileName().toStdString().c_str());
      }
      else
      {
         mButtons[i]->SetShowing(false);
         mPlayButtons[i]->SetShowing(false);
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

void SampleBrowser::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mCurrentDirectory.toStdString();
}

void SampleBrowser::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   std::string currentDirectory;
   in >> currentDirectory;
   SetDirectory(currentDirectory);
}
