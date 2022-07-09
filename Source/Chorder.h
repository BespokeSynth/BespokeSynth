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
//  Chorder.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__Chorder__
#define __modularSynth__Chorder__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "DropdownList.h"
#include "Scale.h"

#define TOTAL_NUM_NOTES 128

class Chorder : public NoteEffectBase, public IDrawableModule, public UIGridListener, public IDropdownListener, public IScaleListener
{
public:
   Chorder();
   virtual ~Chorder();
   static IDrawableModule* Create() { return new Chorder(); }


   void CreateUIControls() override;

   void AddTone(int tone, float velocity = 1);
   void RemoveTone(int tone);

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* dropdown, int oldVal) override;

   void OnScaleChanged() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   virtual bool Enabled() const override { return mEnabled; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 135;
      height = 75;
   }
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void PlayChorderNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);
   void CheckLeftovers();
   void SyncChord();

   UIGrid* mChordGrid;
   int mVelocity;
   bool mInputNotes[TOTAL_NUM_NOTES];
   int mHeldCount[TOTAL_NUM_NOTES];

   bool mDiatonic;
   int mChordIndex;
   int mInversion;
   Checkbox* mDiatonicCheckbox;
   DropdownList* mChordDropdown;
   DropdownList* mInversionDropdown;
};

#endif /* defined(__modularSynth__Chorder__) */
