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
//  NoteVibrato.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#ifndef __Bespoke__NoteVibrato__
#define __Bespoke__NoteVibrato__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "DropdownList.h"
#include "Transport.h"

class NoteVibrato : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IAudioPoller
{
public:
   NoteVibrato();
   virtual ~NoteVibrato();
   static IDrawableModule* Create() { return new NoteVibrato(); }


   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnTransportAdvanced(float amount) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 138;
      height = 22;
   }
   bool Enabled() const override { return mEnabled; }

   NoteInterval mVibratoInterval;
   DropdownList* mIntervalSelector;
   float mVibratoAmount;
   FloatSlider* mVibratoSlider;

   Modulations mModulation;
};

#endif /* defined(__Bespoke__NoteVibrato__) */
