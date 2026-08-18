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
//  BouncingBalls.h
//  Bespoke
//
//  Created by Bespoke.
//
//

#pragma once

#include "Checkbox.h"
#include "Slider.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include <vector>

class BouncingBalls : public IDrawableModule, public INoteSource, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   BouncingBalls();
   virtual ~BouncingBalls();
   static IDrawableModule* Create() { return new BouncingBalls(); }

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
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   // Save/Load
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   void RespawnBalls();
   void TriggerNote(int pitch);
   void Reseed();

   struct Ball
   {
      float x, y;
      float dx, dy;
      int pitch;
   };

   std::vector<Ball> mBalls;

   IntSlider* mNumBallsSlider{ nullptr };
   FloatSlider* mSpeedSlider{ nullptr };
   FloatSlider* mSizeSlider{ nullptr };
   Checkbox* mSymmetryCheckbox{ nullptr };
   IntSlider* mLowsSlider{ nullptr };
   IntSlider* mHighsSlider{ nullptr };
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };

   int mNumBalls{ 5 };
   float mSpeed{ 50.0f };
   float mSize{ 10.0f };
   bool mSymmetry{ false };
   int mLows{ 36 };
   int mHighs{ 84 };
   int mSeed{ 1 };

   float mArenaX{ 0 };
   float mArenaY{ 0 };
   float mArenaW{ 200 };
   float mArenaH{ 200 };

   double mLastTime{ 0 };
};
