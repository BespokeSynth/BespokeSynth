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
//  NoteGate.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#ifndef __Bespoke__NoteGate__
#define __Bespoke__NoteGate__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class NoteGate : public NoteEffectBase, public IDrawableModule
{
public:
   NoteGate();
   virtual ~NoteGate();
   static IDrawableModule* Create() { return new NoteGate(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   bool mGate{ true };
   Checkbox* mGateCheckbox{ nullptr };
   std::array<NoteInputElement, 128> mActiveNotes{ false };
};


#endif /* defined(__Bespoke__NoteGate__) */
