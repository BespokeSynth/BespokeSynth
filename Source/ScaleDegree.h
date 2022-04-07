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
//  ScaleDegree.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/5/16.
//
//

#ifndef __Bespoke__ScaleDegree__
#define __Bespoke__ScaleDegree__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "DropdownList.h"

class ScaleDegree : public NoteEffectBase, public IDrawableModule, public IDropdownListener
{
public:
   ScaleDegree();
   static IDrawableModule* Create() { return new ScaleDegree(); }


   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   struct NoteInfo
   {
      NoteInfo()
      : mOn(false)
      , mVelocity(0)
      , mVoiceIdx(-1)
      {}
      int mOn;
      int mVelocity;
      int mVoiceIdx;
      int mOutputPitch;
   };

   int TransformPitch(int pitch);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;
   int mScaleDegree;
   DropdownList* mScaleDegreeSelector;
   std::array<NoteInfo, 128> mInputNotes;
   Checkbox* mRetriggerCheckbox;
   bool mRetrigger;
};

#endif /* defined(__Bespoke__ScaleDegree__) */
