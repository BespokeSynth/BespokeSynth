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
//  ScaleDetect.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/10/13.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Scale.h"
#include "DropdownList.h"

struct MatchingScale
{
   MatchingScale(int root, std::string type)
   : mRoot(root)
   , mType(type)
   {}
   int mRoot;
   std::string mType;
};

class ScaleDetect : public NoteEffectBase, public IDrawableModule, public IButtonListener, public IDropdownListener
{
public:
   ScaleDetect();
   static IDrawableModule* Create() { return new ScaleDetect(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   bool ScaleSatisfied(int root, std::string type);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 140;
      height = 36;
   }

   std::array<bool, 128> mPitchOn{ false };
   ClickButton* mResetButton{ nullptr };
   int mLastPitch{ 0 };
   bool mDoDetect{ true };
   bool mNeedsUpdate{ false };

   DropdownList* mMatchesDropdown{ nullptr };
   int mSelectedMatch{ 0 };
};
