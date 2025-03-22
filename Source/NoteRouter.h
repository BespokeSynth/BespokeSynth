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
//  NoteRouter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/24/13.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "RadioButton.h"

class NoteRouter : public NoteEffectBase, public IDrawableModule, public IRadioButtonListener
{
public:
   NoteRouter();
   static IDrawableModule* Create() { return new NoteRouter(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void SetActiveIndex(int index) { mRouteMask = 1 << index; }
   void SetSelectedMask(int mask);

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   bool IsIndexActive(int idx) const;

   int mRouteMask{ 0 };
   RadioButton* mRouteSelector{ nullptr };
   std::vector<AdditionalNoteCable*> mDestinationCables;
   bool mRadioButtonMode{ false };
   bool mOnlyShowActiveCables{ false };
};
