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
//  NoteTransformer.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/13/14.
//
//

#ifndef __modularSynth__NoteTransformer__
#define __modularSynth__NoteTransformer__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class NoteTransformer : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   NoteTransformer();
   ~NoteTransformer();
   static IDrawableModule* Create() { return new NoteTransformer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 135;
   }

   int mToneMod[7]{};
   IntSlider* mToneModSlider[7];
   double mLastTimeTonePlayed[7]{};
   int mLastNoteOnForPitch[127];
};


#endif /* defined(__modularSynth__NoteTransformer__) */
