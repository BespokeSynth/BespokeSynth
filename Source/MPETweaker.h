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
#include "ModulationChain.h"

class MPETweaker : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   MPETweaker();
   virtual ~MPETweaker();
   static IDrawableModule* Create() { return new MPETweaker(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };

   float mPitchBendMultiplier{ 1 };
   FloatSlider* mPitchBendMultiplierSlider{ nullptr };
   float mPitchBendOffset{ 0 };
   FloatSlider* mPitchBendOffsetSlider{ nullptr };
   float mPressureMultiplier{ 1 };
   FloatSlider* mPressureMultiplierSlider{ nullptr };
   float mPressureOffset{ 0 };
   FloatSlider* mPressureOffsetSlider{ nullptr };
   float mModWheelMultiplier{ 1 };
   FloatSlider* mModWheelMultiplierSlider{ nullptr };
   float mModWheelOffset{ 0 };
   FloatSlider* mModWheelOffsetSlider{ nullptr };

   Modulations mModulationMult{ true };
   Modulations mModulationOffset{ true };
};
