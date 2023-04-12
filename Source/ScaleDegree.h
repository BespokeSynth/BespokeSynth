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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   struct NoteInfo
   {
      int mOn{ false };
      int mVelocity{ 0 };
      int mVoiceIdx{ -1 };
      int mOutputPitch{ 0 };
   };

   int TransformPitch(int pitch);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };
   int mScaleDegree{ 0 };
   DropdownList* mScaleDegreeSelector{ nullptr };
   std::array<NoteInfo, 128> mInputNotes{};
   Checkbox* mRetriggerCheckbox{ nullptr };
   bool mRetrigger{ false };
   Checkbox* mDiatonicCheckbox{ nullptr };
   bool mDiatonic{ true };
};

#endif /* defined(__Bespoke__ScaleDegree__) */
