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
//  StutterControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/25/15.
//
//

#pragma once

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Checkbox.h"
#include "Stutter.h"
#include "Slider.h"
#include "IAudioProcessor.h"
#include "GridController.h"
#include "INoteReceiver.h"
#include "Push2Control.h"

class StutterControl : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IGridControllerListener, public INoteReceiver, public IPush2GridController
{
public:
   StutterControl();
   ~StutterControl();
   static IDrawableModule* Create() { return new StutterControl(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   //IAudioSource
   void Process(double time) override;

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx) override {}

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   enum StutterType
   {
      kHalf,
      kQuarter,
      k8th,
      k16th,
      k32nd,
      k64th,
      kReverse,
      kRampIn,
      kRampOut,
      kTumbleUp,
      kTumbleDown,
      kHalfSpeed,
      kDoubleSpeed,
      kDotted8th,
      kQuarterTriplets,
      kFree,
      kNumStutterTypes
   };

   StutterType GetStutterFromKey(int key);
   void SendStutter(double time, StutterParams stutter, bool on);
   StutterParams GetStutter(StutterType type);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void UpdateGridLights();

   Stutter mStutterProcessor;
   Checkbox* mStutterCheckboxes[StutterType::kNumStutterTypes]{ nullptr };
   bool mStutter[StutterType::kNumStutterTypes]{};
   FloatSlider* mFreeLengthSlider{ nullptr };
   FloatSlider* mFreeSpeedSlider{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
};
