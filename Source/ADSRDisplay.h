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
//  ADSRDisplay.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/28/13.
//
//

#pragma once

#include "IUIControl.h"
#include "ADSR.h"
#include "Slider.h"

class IDrawableModule;
class EnvelopeEditor;

class ADSRDisplay : public IUIControl
{
public:
   ADSRDisplay(IDrawableModule* owner, const char* name, int x, int y, int w, int h, ::ADSR* adsr);
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void SetVol(float vol) { mVol = vol; }
   void SetHighlighted(bool highlighted) { mHighlighted = highlighted; }
   float GetMaxTime() const { return mMaxTime; }
   float& GetMaxTime() { return mMaxTime; }
   void SetMaxTime(float maxTime);
   void SetADSR(::ADSR* adsr);
   ::ADSR* GetADSR() { return mAdsr; }
   void SpawnEnvelopeEditor();
   void SetOverrideDrawTime(double time) { mOverrideDrawTime = time; }
   void SetDimensions(float w, float h)
   {
      mWidth = w;
      mHeight = h;
   }
   void SetShowing(bool showing) override
   {
      IUIControl::SetShowing(showing);
      UpdateSliderVisibility();
   }
   FloatSlider* GetASlider() { return mASlider; }
   FloatSlider* GetDSlider() { return mDSlider; }
   FloatSlider* GetSSlider() { return mSSlider; }
   FloatSlider* GetRSlider() { return mRSlider; }

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override {}
   void SetValue(float value, double time, bool forceUpdate = false) override {}
   bool CanBeTargetedBy(PatchCableSource* source) const override { return false; }
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool GetNoHover() const override { return true; }

   enum DisplayMode
   {
      kDisplayEnvelope,
      kDisplaySliders
   };
   static void ToggleDisplayMode();

protected:
   ~ADSRDisplay(); //protected so that it can't be created on the stack

private:
   enum AdjustParam
   {
      kAdjustAttack,
      kAdjustDecaySustain,
      kAdjustRelease,
      kAdjustEnvelopeEditor,
      kAdjustNone,
      kAdjustAttackAR,
      kAdjustReleaseAR,
      kAdjustViewLength
   } mAdjustMode{ AdjustParam::kAdjustNone };

   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void UpdateSliderVisibility();
   ofVec2f GetDrawPoint(float time, const ADSR::EventInfo& adsrEvent);

   float mWidth;
   float mHeight;
   float mVol{ 1 };
   float mMaxTime{ 1000 };
   bool mClick{ false };
   ::ADSR* mAdsr;
   ofVec2f mClickStart;
   ::ADSR mClickAdsr;
   float mClickLength{ 1000 };
   bool mHighlighted{ false };
   FloatSlider* mASlider{ nullptr };
   FloatSlider* mDSlider{ nullptr };
   FloatSlider* mSSlider{ nullptr };
   FloatSlider* mRSlider{ nullptr };
   static DisplayMode sDisplayMode;
   EnvelopeEditor* mEditor{ nullptr };
   double mOverrideDrawTime{ -1 };
   std::array<double, 10> mDrawTimeHistory{};
   int mDrawTimeHistoryIndex{ 0 };
};
