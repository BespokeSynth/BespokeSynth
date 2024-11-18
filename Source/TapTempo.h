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
//  TapTempo.h
//  Bespoke
//
//  Created by Andrius Merkys on 10/25/24.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "IModulator.h"
#include "INoteReceiver.h"
#include "IPulseReceiver.h"
#include "TextEntry.h"

class PatchCableSource;

class TapTempo : public IDrawableModule, public INoteReceiver, public IPulseReceiver, public ITextEntryListener, public IButtonListener, public IModulator
{
public:
   TapTempo();
   virtual ~TapTempo();
   static IDrawableModule* Create() { return new TapTempo(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void TextEntryComplete(TextEntry* entry) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   void OnPulse(double time, float velocity, int flags) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   TextEntry* mWindowEntry{ nullptr };
   ClickButton* mReset{ nullptr };

   int mCount{ 0 };
   int mWindow{ 4 };
   std::array<float, mWindow> mBeats;

   float mWidth{ 200 };
   float mHeight{ 20 };
};
