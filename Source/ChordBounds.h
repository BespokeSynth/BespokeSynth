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

    ChordBounds.h
    Created: 4 Jan 2024 5:31:53pm
    Author:  Andrius Merkys

  ==============================================================================
*/

#ifndef __Bespoke__ChordBounds__
#define __Bespoke__ChordBounds__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"

class ChordBounds : public INoteReceiver, public INoteSource, public IDrawableModule, public IAudioPoller
{
public:
   ChordBounds();
   virtual ~ChordBounds();
   static IDrawableModule* Create() { return new ChordBounds(); }
   static bool AcceptsNotes() { return true; }

   void CreateUIControls() override;
   void Init() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 90;
      height = 0;
   }
   bool IsEnabled() const override { return true; }

   AdditionalNoteCable* mPatchCableSource2{ nullptr };
   std::array<NoteMessage, 128> mActiveNotes{};
   int mNoteMin = -1;
   int mNoteMax = -1;
};

#endif /* defined(__Bespoke__ChordBounds__) */
