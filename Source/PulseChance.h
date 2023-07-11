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

    PulseChance.h
    Created: 4 Feb 2020 12:17:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "ClickButton.h"
#include "TextEntry.h"

class PulseChance : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   PulseChance();
   virtual ~PulseChance();
   static IDrawableModule* Create() { return new PulseChance(); }
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

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void Reseed();

   float mChance{ 1 };
   FloatSlider* mChanceSlider{ nullptr };
   float mLastRejectTime{ 0 };
   float mLastAcceptTime{ 0 };
   bool mDeterministic{ false };
   Checkbox* mDeterministicCheckbox{ nullptr };
   int mSeed{ 0 };
   int mRandomIndex{ 0 };
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };
};
