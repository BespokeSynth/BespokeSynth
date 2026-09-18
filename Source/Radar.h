/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2023 Ryan Challinor (contact: awwbees@gmail.com)

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
//  Radar.h
//  Bespoke
//
//  Created by Bespoke.
//
//

#pragma once

#include "Slider.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include <vector>

class Radar : public IDrawableModule, public INoteSource, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   Radar();
   virtual ~Radar();
   static IDrawableModule* Create() { return new Radar(); }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void CreateUIControls() override;
   void Init() override;

   // IDrawableModule
   void Poll() override;
   void DrawModule() override;
   void Resize(float w, float h) override;

   // Listeners
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void ButtonClicked(ClickButton* button, double time) override;

   // Save/Load
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   void RecalculateTriggers();
   void Reseed();

   struct TriggerPoint
   {
      float angle;
      float distance;
      int pitch;
      float phase; // 0.0 to 1.0 (relative to mRadarSize)
      float flashAmount; // 0.0 to 1.0 for visual feedback
   };

   std::vector<TriggerPoint> mTriggers;

   FloatSlider* mRadarSizeSlider{ nullptr };
   FloatSlider* mRadarDurationSlider{ nullptr };
   IntSlider* mNumRadarsSlider{ nullptr };
   IntSlider* mNumNotesSlider{ nullptr };
   IntSlider* mLowsSlider{ nullptr };
   IntSlider* mHighsSlider{ nullptr };
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };

   float mRadarSize{ 100.0f };
   float mRadarDuration{ 1000.0f }; // milliseconds
   int mNumRadars{ 1 };
   int mNumNotes{ 10 };
   int mLows{ 36 };
   int mHighs{ 84 };
   int mSeed{ 1 };

   float mArenaX{ 0 };
   float mArenaY{ 0 };
   float mArenaW{ 220 };
   float mArenaH{ 220 };

   float mPhase{ 0 };
   double mLastTime{ 0 };
};
