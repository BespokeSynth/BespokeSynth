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
//
//  VelocityStepSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/14/14.
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
#include "MidiController.h"

#define VSS_MAX_STEPS 8
#define VSS_RANGE 127

class VelocityStepSequencer : public IDrawableModule, public ITimeListener, public NoteEffectBase, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public MidiDeviceListener
{
public:
   VelocityStepSequencer();
   ~VelocityStepSequencer();
   static IDrawableModule* Create() { return new VelocityStepSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetMidiController(std::string name);

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 160;
      height = 160;
   }

   int mVels[VSS_MAX_STEPS]{};
   IntSlider* mVelSliders[VSS_MAX_STEPS]{};

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   int mArpIndex{ -1 };

   DropdownList* mIntervalSelector{ nullptr };
   bool mResetOnDownbeat{ true };
   Checkbox* mResetOnDownbeatCheckbox{ nullptr };
   int mCurrentVelocity{ 80 };

   int mLength{ VSS_MAX_STEPS };
   IntSlider* mLengthSlider{ nullptr };

   MidiController* mController{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
