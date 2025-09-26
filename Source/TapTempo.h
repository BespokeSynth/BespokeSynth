/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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

    TapTempo.h
    Created: 26 Sep 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "ClickButton.h"

class TapTempo : public IDrawableModule, public IPulseReceiver, public IButtonListener, public IKeyboardFocusListener
{
public:
   TapTempo();
   virtual ~TapTempo();
   static IDrawableModule* Create() { return new TapTempo(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool HasDebugDraw() const override { return true; }

   void OnKeyPressed(int key, bool isRepeat) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;

   void CalculateTempo();

   ClickButton* mTapButton{ nullptr };
   ClickButton* mSendTempoButton{ nullptr };
   bool mRoundTempo{ true };
   Checkbox* mRoundTempoCheckbox{ nullptr };

   std::array<double, 14> mTimeSamples{};
   int mNumAccumulatedSamples{ 0 };
   int mLastTimeSamplesWriteIdx{ 0 };
   float mLastCalculatedTempo{ 120.0f };
   float mStandardDeviation{ 0.0f };
};
