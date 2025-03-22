/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2023 Ryan Challinor (contact: awwbees@gmail.com)

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
//
//  RhythmSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/5/23.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "INoteReceiver.h"
#include "IPulseReceiver.h"
#include "IDrivableSequencer.h"

class RhythmSequencer : public IDrawableModule, public ITimeListener, public NoteEffectBase, public IPulseReceiver, public IDrivableSequencer, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener
{
public:
   RhythmSequencer();
   ~RhythmSequencer();
   static IDrawableModule* Create() { return new RhythmSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   void Step(double time, float velocity, int pulseFlags);
   int GetArpIndex(double time, int current, int length, int pulseFlags);
   bool DoesStepHold(int index, int depth) const;

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mWidth{ 160 };
   float mHeight{ 160 };

   bool mHasExternalPulseSource{ false };

   static constexpr int kMaxSteps = 8;

   enum class StepAction
   {
      On,
      Hold,
      Off
   };

   struct StepData
   {
      StepAction mAction{ StepAction::On };
      DropdownList* mActionSelector{ nullptr };
      float mVel{ 1.0f };
      FloatSlider* mVelSlider{ nullptr };
      int mOctave{ 0 };
      IntSlider* mOctaveSlider{ nullptr };
      int mDegree{ 0 };
      DropdownList* mDegreeSelector{ nullptr };
   };

   std::array<StepData, kMaxSteps> mStepData{};

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   int mArpIndex{ -1 };
   int mArpIndexAction{ -1 };
   int mArpIndexVel{ -1 };
   int mArpIndexOctave{ -1 };
   int mArpIndexDegree{ -1 };

   DropdownList* mIntervalSelector{ nullptr };

   int mLength{ kMaxSteps };
   int mLengthAction{ kMaxSteps };
   int mLengthVel{ kMaxSteps };
   int mLengthOctave{ kMaxSteps };
   int mLengthDegree{ kMaxSteps };
   IntSlider* mLengthSlider{ nullptr };
   IntSlider* mLengthActionSlider{ nullptr };
   IntSlider* mLengthVelSlider{ nullptr };
   IntSlider* mLengthOctaveSlider{ nullptr };
   IntSlider* mLengthDegreeSlider{ nullptr };
   bool mLinkLengths{ true };
   Checkbox* mLinkLengthsCheckbox{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };

   std::array<bool, 128> mInputPitches{};
};
