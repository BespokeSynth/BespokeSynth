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
//  ModulationVisualizer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#ifndef __Bespoke__ModulationVisualizer__
#define __Bespoke__ModulationVisualizer__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

struct ModulationParameters;

class ModulationVisualizer : public NoteEffectBase, public IDrawableModule
{
public:
   ModulationVisualizer();
   static IDrawableModule* Create() { return new ModulationVisualizer(); }
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
      width = 300;
      height = 100;
   }

   struct VizVoice
   {
      std::string GetInfoString();
      bool mActive{ false };
      ModulationParameters mModulators;
   };

   VizVoice mGlobalModulation;
   std::vector<VizVoice> mVoices;
};

#endif /* defined(__Bespoke__ModulationVisualizer__) */
