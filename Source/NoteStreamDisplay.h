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
/*
  ==============================================================================

    NoteStreamDisplay.h
    Created: 21 May 2020 11:13:12pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "ClickButton.h"

class NoteStreamDisplay : public NoteEffectBase, public IDrawableModule, public IButtonListener
{
public:
   NoteStreamDisplay();
   static IDrawableModule* Create() { return new NoteStreamDisplay(); }
   void CreateUIControls() override;
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void ButtonClicked(ClickButton* button, double time) override;

   bool HasDebugDraw() const override { return true; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }
   bool IsElementActive(int index) const;
   float GetYPos(int pitch, float noteHeight) const;

   struct NoteStreamElement
   {
      int pitch{ 0 };
      int velocity{ 0 };
      double timeOn{ -1 };
      double timeOff{ -1 };
   };

   static const int kNoteStreamCapacity = 100;
   NoteStreamElement mNoteStream[kNoteStreamCapacity];
   float mWidth{ 400 };
   float mHeight{ 200 };
   float mDurationMs{ 2000 };
   int mPitchMin{ 127 };
   int mPitchMax{ 0 };
   ClickButton* mResetButton{ nullptr };
};
