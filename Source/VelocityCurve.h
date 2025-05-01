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
//  VelocityCurve.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/17/22.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "EnvelopeEditor.h"

class VelocityCurve : public NoteEffectBase, public IDrawableModule
{
public:
   VelocityCurve();
   static IDrawableModule* Create() { return new VelocityCurve(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 106;
      h = 105;
   }

   void OnClicked(float x, float y, bool right) override;

   EnvelopeControl mEnvelopeControl{ ofVec2f{ 3, 3 }, ofVec2f{ 100, 100 } };
   ::ADSR mAdsr;
   float mLastInputVelocity{ 0 };
   double mLastInputTime{ -9999 };
};
