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
//  PitchToValue.h
//  Bespoke
//
//  Created by Andrius Merkys on 12/20/23.
//
//

#ifndef __Bespoke__PitchToValue__
#define __Bespoke__PitchToValue__

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IModulator.h"

class PatchCableSource;
class IUIControl;

class PitchToValue : public IDrawableModule, public INoteReceiver, public IModulator
{
public:
   PitchToValue();
   virtual ~PitchToValue();
   static IDrawableModule* Create() { return new PitchToValue(); }
   static bool AcceptsNotes() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 110;
      height = 0;
   }
   bool IsEnabled() const override { return mEnabled; }

   int mValue;
};

#endif /* defined(__Bespoke__PitchToValue__) */
