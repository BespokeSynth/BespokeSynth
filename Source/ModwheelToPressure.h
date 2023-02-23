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
//  ModwheelToPressure.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__ModwheelToPressure__
#define __Bespoke__ModwheelToPressure__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ModulationChain.h"

class ModwheelToPressure : public NoteEffectBase, public IDrawableModule
{
public:
   ModwheelToPressure();
   virtual ~ModwheelToPressure();
   static IDrawableModule* Create() { return new ModwheelToPressure(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 0;
   }
};

#endif /* defined(__Bespoke__ModwheelToPressure__) */
