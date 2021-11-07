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
//  NoteToggle.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/02/2021
//
//

#pragma once

#include "IDrawableModule.h"
#include "INoteReceiver.h"

class PatchCableSource;

class NoteToggle : public IDrawableModule, public INoteReceiver
{
public:
   NoteToggle();
   ~NoteToggle();
   static IDrawableModule* Create() { return new NoteToggle(); }
   
   void CreateUIControls() override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   std::array<bool, 128> mHeldPitches{ false };
   PatchCableSource* mControlCable;
   IUIControl* mControlTarget;
};
