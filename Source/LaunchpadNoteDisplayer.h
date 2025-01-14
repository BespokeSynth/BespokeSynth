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
//  LaunchpadNoteDisplayer.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"

class LaunchpadKeyboard;

class LaunchpadNoteDisplayer : public NoteEffectBase, public IDrawableModule
{
public:
   LaunchpadNoteDisplayer();
   static IDrawableModule* Create() { return new LaunchpadNoteDisplayer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void SetLaunchpad(LaunchpadKeyboard* launchpad) { mLaunchpad = launchpad; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 80;
      height = 0;
   }

   LaunchpadKeyboard* mLaunchpad{ nullptr };
};
