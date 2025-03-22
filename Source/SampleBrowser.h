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

    SampleBrowser.h
    Created: 19 Jun 2021 6:46:36pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Sample.h"
#include "ClickButton.h"
#include "IAudioSource.h"

class SampleBrowser : public IDrawableModule, public IButtonListener, public IAudioSource
{
public:
   SampleBrowser();
   ~SampleBrowser();
   static IDrawableModule* Create() { return new SampleBrowser(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;

   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 300;
      height = 38 + (int)mButtons.size() * 17;
   }

   void SetDirectory(juce::String dirPath);
   int GetNumPages() const;
   void ShowPage(int page);
   bool IsSamplePlaying(int index) const;

   juce::String mCurrentDirectory;
   juce::StringArray mDirectoryListing;
   std::array<ClickButton*, 30> mButtons{ nullptr };
   std::array<ClickButton*, 30> mPlayButtons{ nullptr };
   ClickButton* mBackButton{ nullptr };
   ClickButton* mForwardButton{ nullptr };
   int mCurrentPage{ 0 };
   Sample mPlayingSample;
   ofMutex mSampleMutex;
};
