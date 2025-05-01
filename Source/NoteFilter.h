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
//  NoteFilter.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#pragma once

#include "NoteEffectBase.h"

class NoteFilter : public NoteEffectBase, public IDrawableModule
{
public:
   NoteFilter();
   virtual ~NoteFilter();
   static IDrawableModule* Create() { return new NoteFilter(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   std::array<bool, 128> mGate;
   std::array<float, 128> mLastPlayTime;
   std::vector<Checkbox*> mGateCheckboxes;
   int mMinPitch{ 0 };
   int mMaxPitch{ 7 };
};
