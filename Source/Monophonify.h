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
//  Monophonify.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#ifndef __modularSynth__Monophonify__
#define __modularSynth__Monophonify__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ModulationChain.h"
#include "DropdownList.h"

class Monophonify : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IDropdownListener
{
public:
   Monophonify();
   static IDrawableModule* Create() { return new Monophonify(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   int GetMostRecentCurrentlyHeldPitch() const;

   double mHeldNotes[128];
   int mInitialPitch{ -1 };
   int mLastPlayedPitch{ -1 };
   int mLastVelocity{ 0 };
   float mWidth{ 200 };
   float mHeight{ 20 };
   int mVoiceIdx{ 0 };

   enum class PortamentoMode
   {
      kAlways,
      kRetriggerHeld,
      kBendHeld
   };

   PortamentoMode mPortamentoMode{ PortamentoMode::kAlways };
   DropdownList* mPortamentoModeSelector{ nullptr };
   float mGlideTime{ 0 };
   FloatSlider* mGlideSlider{ nullptr };
   ModulationChain mPitchBend{ 0 };
};


#endif /* defined(__modularSynth__Monophonify__) */
