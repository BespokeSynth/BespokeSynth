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
//  NoteCreator.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#pragma once

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "Slider.h"
#include "IPulseReceiver.h"

class NoteCreator : public IDrawableModule, public INoteSource, public IButtonListener, public ITextEntryListener, public IFloatSliderListener, public IPulseReceiver
{
public:
   NoteCreator();
   virtual ~NoteCreator();
   static IDrawableModule* Create() { return new NoteCreator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnPulse(double time, float velocity, int flags) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

protected:
   void TriggerNote(double time, float velocity);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   int mWidth{ 200 };
   int mHeight{ 20 };

   ClickButton* mTriggerButton{ nullptr };
   TextEntry* mPitchEntry{ nullptr };
   FloatSlider* mVelocitySlider{ nullptr };
   FloatSlider* mDurationSlider{ nullptr };
   Checkbox* mNoteOnCheckbox{ nullptr };
   int mPitch{ 48 };
   float mVelocity{ 1 };
   float mDuration{ 100 };
   double mStartTime{ 0 };
   bool mNoteOn{ false };
   int mVoiceIndex{ -1 };
};
