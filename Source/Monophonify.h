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
#include "Checkbox.h"
#include "Slider.h"
#include "ModulationChain.h"

class Monophonify : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   Monophonify();
   static IDrawableModule* Create() { return new Monophonify(); }
   
   std::string GetTitleLabel() override { return "portamento"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   int GetMostRecentPitch() const;
   
   double mHeldNotes[128];
   int mInitialPitch;
   int mLastPlayedPitch;
   int mLastVelocity;
   float mWidth;
   float mHeight;
   int mVoiceIdx;
   
   bool mRequireHeldNote;
   Checkbox* mRequireHeldNoteCheckbox;
   float mGlideTime;
   FloatSlider* mGlideSlider;
   ModulationChain mPitchBend;
};


#endif /* defined(__modularSynth__Monophonify__) */

