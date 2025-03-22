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

    ChordHold.h
    Created: 3 Mar 2021 9:56:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "IPulseReceiver.h"

class ChordHolder : public NoteEffectBase, public IDrawableModule, public IButtonListener, public IPulseReceiver
{
public:
   ChordHolder();
   static IDrawableModule* Create() { return new ChordHolder(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 131;
      height = 21;
   }

   void Stop(double time);

   std::array<bool, 128> mNoteInputHeld{ false };
   std::array<bool, 128> mNotePlaying{ false };

   ClickButton* mStopButton{ nullptr };
   bool mOnlyPlayWhenPulsed{ false };
   Checkbox* mOnlyPlayWhenPulsedCheckbox{ nullptr };
};
