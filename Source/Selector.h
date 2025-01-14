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
//  Selector.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#pragma once

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "INoteReceiver.h"

class PatchCableSource;

class Selector : public IDrawableModule, public IRadioButtonListener, public INoteReceiver
{
public:
   Selector();
   ~Selector() override;
   static IDrawableModule* Create() { return new Selector(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void SyncList();
   void SetIndex(int index, double time);

   RadioButton* mSelector{ nullptr };
   int mCurrentValue{ 0 };

   std::vector<PatchCableSource*> mControlCables;
};
