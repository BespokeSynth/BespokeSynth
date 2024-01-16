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

    NoteMinMax.h
    Created: 4 Jan 2024 5:31:53pm
    Author:  Andrius Merkys

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"

class NoteMinMax : public INoteReceiver, public INoteSource, public IDrawableModule
{
public:
   NoteMinMax();
   static IDrawableModule* Create() { return new NoteMinMax(); }
   static bool AcceptsNotes() { return true; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 40; };
   bool Enabled() const override { return true; }

   AdditionalNoteCable* mDestinationCables[2];

   std::array<bool, 128> mNotePlaying{ false };
   std::array<int, 128> mVelocityPlaying{ 0 };
   std::array<int, 128> mVoiceIdxPlaying{ 0 };
   ModulationParameters mModulationParametersPlaying[128];
};
