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
//  SustainPedal.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/7/14.
//
//

#ifndef __Bespoke__SustainPedal__
#define __Bespoke__SustainPedal__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"

class SustainPedal : public NoteEffectBase, public IDrawableModule
{
public:
   SustainPedal();
   static IDrawableModule* Create() { return new SustainPedal(); }
   
   
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 21; }
   bool Enabled() const override { return true; }
   
   std::array<bool, 128> mIsNoteBeingSustained{ false };
   bool mSustain;
   Checkbox* mSustainCheckbox;
};

#endif /* defined(__Bespoke__SustainPedal__) */
