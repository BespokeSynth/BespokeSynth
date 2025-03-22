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

#pragma once

#include "Transport.h"
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
   void OnClicked(float x, float y, bool right);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void CreateUIControls();
   void OnResize();
   void UpdateGrid();

   int mIndex{ 0 };
   UIGrid* mGrid{ nullptr };
   int mLength{ 4 };
   DropdownList* mLengthSelector{ nullptr };
   int mPitch{ 0 };
   TextEntry* mNoteSelector{ nullptr };
   Polyrhythms* mOwner{ nullptr };
};

class Polyrhythms : public INoteSource, public IDrawableModule, public IAudioPoller, public IDropdownListener, public ITextEntryListener
{
public:
   Polyrhythms();
   ~Polyrhythms();
   static IDrawableModule* Create() { return new Polyrhythms(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

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

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void LoadState(FileStreamIn& in, int rev) override;
   void SaveState(FileStreamOut& out) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;

   float mWidth{ 350 };
   float mHeight;
   std::array<RhythmLine*, 8> mRhythmLines;
};
