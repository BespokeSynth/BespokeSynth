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

    MPETweaker.h
    Created: 6 Aug 2021 9:11:11pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"

class MPETweaker : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   MPETweaker();
   virtual ~MPETweaker();
   static IDrawableModule* Create() { return new MPETweaker(); }
   
   std::string GetTitleLabel() override { return "mpe tweaker"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   
   float mPitchBendMultiplier;
   FloatSlider* mPitchBendMultiplierSlider;
   float mPitchBendOffset;
   FloatSlider* mPitchBendOffsetSlider;
   float mPressureMultiplier;
   FloatSlider* mPressureMultiplierSlider;
   float mPressureOffset;
   FloatSlider* mPressureOffsetSlider;
   float mModWheelMultiplier;
   FloatSlider* mModWheelMultiplierSlider;
   float mModWheelOffset;
   FloatSlider* mModWheelOffsetSlider;
   
   Modulations mModulationMult;
   Modulations mModulationOffset;
};
