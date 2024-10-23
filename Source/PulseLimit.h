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

    PulseLimit.h
    Created: 22 Oct 2024 11:32:00am
    Author:  Andrius Merkys

  ==============================================================================
*/

#pragma once
#include "ClickButton.h"
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "TextEntry.h"

class PulseLimit : public IDrawableModule, public IPulseSource, public IPulseReceiver, public ITextEntryListener, public IButtonListener
{
public:
   PulseLimit();
   virtual ~PulseLimit();
   static IDrawableModule* Create() { return new PulseLimit(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   int mLimit{ 0 };
   int mCount{ 0 };
   TextEntry* mLimitEntry{ nullptr };
   ClickButton* mResetButton{ nullptr };

   float mWidth{ 200 };
   float mHeight{ 20 };
};
