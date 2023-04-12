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

    NoteCounter.h
    Created: 24 Apr 2021 3:47:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IPulseReceiver.h"
#include "IDrivableSequencer.h"
#include "ClickButton.h"
#include "TextEntry.h"

class NoteCounter : public IDrawableModule, public INoteSource, public ITimeListener, public IIntSliderListener, public IDropdownListener, public IPulseReceiver, public IDrivableSequencer, public ITextEntryListener, public IButtonListener
{
public:
   NoteCounter();
   ~NoteCounter();
   static IDrawableModule* Create() { return new NoteCounter(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void Step(double time, float velocity, int pulseFlags);
   void Reseed();
   std::uint64_t GetRandom(double time, int seedOffset) const;

   float mWidth{ 200 };
   float mHeight{ 20 };
   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   int mStart{ 0 };
   IntSlider* mStartSlider{ nullptr };
   int mLength{ 16 };
   IntSlider* mLengthSlider{ nullptr };
   int mStep{ 0 };
   Checkbox* mSyncCheckbox{ nullptr };
   bool mSync{ false };
   int mCustomDivisor{ 8 };
   IntSlider* mCustomDivisorSlider{ nullptr };
   bool mRandom{ false };
   Checkbox* mRandomCheckbox{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };

   bool mHasExternalPulseSource{ false };

   bool mDeterministic{ false };
   int mDeterministicLength{ 4 };
   IntSlider* mDeterministicLengthSlider{ nullptr };
   int mSeed{ 0 };
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };
};
