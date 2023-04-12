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

    PulseHocket.h
    Created: 22 Feb 2020 10:40:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "TextEntry.h"
#include "ClickButton.h"

class PulseHocket : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   PulseHocket();
   virtual ~PulseHocket();
   static IDrawableModule* Create() { return new PulseHocket(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void Reseed();
   void AdjustHeight();

   static const int kMaxDestinations = 16;
   int mNumDestinations{ 5 };
   float mWeight[kMaxDestinations]{};
   FloatSlider* mWeightSlider[kMaxDestinations]{};
   std::vector<PatchCableSource*> mDestinationCables;
   float mWidth{ 200 };
   float mHeight{ 20 };
   bool mDeterministic{ false };
   int mSeed{ 0 };
   int mRandomIndex{ 0 };
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };
};
