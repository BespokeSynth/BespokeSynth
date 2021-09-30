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

#ifndef __modularSynth__ScaleDetect__
#define __modularSynth__ScaleDetect__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Scale.h"
#include "DropdownList.h"

struct MatchingScale
{
   MatchingScale(int root, std::string type) : mRoot(root), mType(type) {}
   int mRoot;
   std::string mType;
};

class ScaleDetect : public NoteEffectBase, public IDrawableModule, public IButtonListener, public IDropdownListener
{
public:
   ScaleDetect();
   static IDrawableModule* Create() { return new ScaleDetect(); }
   
   std::string GetTitleLabel() override { return "detect"; }
   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   bool ScaleSatisfied(int root, std::string type);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 140; height = 36; }
   bool Enabled() const override { return true; }

   std::array<bool,128> mPitchOn{false};
   ClickButton* mResetButton;
   int mLastPitch;
   bool mDoDetect;
   bool mNeedsUpdate;
   
   DropdownList* mMatchesDropdown;
   int mSelectedMatch;
};

#endif /* defined(__modularSynth__ScaleDetect__) */

