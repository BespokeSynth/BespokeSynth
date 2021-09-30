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
//  Polyrhythms.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/12/13.
//
//

#ifndef __modularSynth__Polyrhythms__
#define __modularSynth__Polyrhythms__

#include <iostream>
#include "Transport.h"
#include "UIGrid.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"

class Polyrhythms;

class RhythmLine
{
public:
   RhythmLine(Polyrhythms* owner, int index);
   void Draw();
   void OnClicked(int x, int y, bool right);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void CreateUIControls();
   void OnResize();
   void UpdateGrid();
   
   int mIndex;
   UIGrid* mGrid;
   int mLength;
   DropdownList* mLengthSelector;
   int mPitch;
   TextEntry* mNoteSelector;
   Polyrhythms* mOwner;
};

class Polyrhythms : public INoteSource, public IDrawableModule, public IAudioPoller, public IDropdownListener, public ITextEntryListener
{
public:
   Polyrhythms();
   ~Polyrhythms();
   static IDrawableModule* Create() { return new Polyrhythms(); }
   
   std::string GetTitleLabel() override { return "polyrhythms"; }
   void CreateUIControls() override;
   void Init() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   void SetEnabled(bool on) override { mEnabled = on; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void LoadState(FileStreamIn& in) override;
   virtual void SaveState(FileStreamOut& out) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=mWidth; height=mHeight; }
   void OnClicked(int x, int y, bool right) override;
   bool Enabled() const override { return mEnabled; }
   
   int mNumLines;
   float mWidth;
   float mHeight;
   std::array<RhythmLine*,8> mRhythmLines;
};

#endif /* defined(__modularSynth__Polyrhythms__) */

