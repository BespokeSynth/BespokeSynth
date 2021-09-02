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

#ifndef __Bespoke__StutterControl__
#define __Bespoke__StutterControl__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Checkbox.h"
#include "Stutter.h"
#include "Slider.h"
#include "IAudioProcessor.h"
#include "GridController.h"
#include "INoteReceiver.h"

class StutterControl : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IGridControllerListener, public INoteReceiver
{
public:
   StutterControl();
   ~StutterControl();
   static IDrawableModule* Create() { return new StutterControl(); }
   
   string GetTitleLabel() override { return "stutter"; }
   void CreateUIControls() override;
   void Init() override;
   
   //IAudioSource
   void Process(double time) override;
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation) override;
   void SendCC(int control, int value, int voiceIdx) override {}
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
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
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   void UpdateGridLights();
   
   Stutter mStutterProcessor;
   Checkbox* mStutterCheckboxes[kNumStutterTypes];
   bool mStutter[kNumStutterTypes];
   FloatSlider* mFreeLengthSlider;
   FloatSlider* mFreeSpeedSlider;
   GridControlTarget* mGridControlTarget;
};

#endif /* defined(__Bespoke__StutterControl__) */
