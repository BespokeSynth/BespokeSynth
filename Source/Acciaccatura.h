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
//  Acciaccatura.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/25/25.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ModulationChain.h"
#include "Transport.h"
#include "DropdownList.h"

class Acciaccatura : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   Acciaccatura();
   virtual ~Acciaccatura();
   static IDrawableModule* Create() { return new Acciaccatura(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

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

   enum class NoteMode
   {
      Scale,
      Chromatic
   };

   float mWidth;
   float mHeight;

   int mOffset{ -1 };
   IntSlider* mOffsetSlider{ nullptr };
   NoteMode mNoteMode{ NoteMode::Scale };
   DropdownList* mNoteModeDropdown{ nullptr };
   float mHoldTimeMs{ 35.0f };
   FloatSlider* mHoldTimeSlider{ nullptr };
   float mGlideTimeMs{ 5.0f };
   FloatSlider* mGlideTimeSlider{ nullptr };

   Modulations mModulation{ false };
};
