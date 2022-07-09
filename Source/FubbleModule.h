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

    FubbleModule.h
    Created: 8 Aug 2020 10:03:56am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Curve.h"
#include "Checkbox.h"
#include "IModulator.h"
#include "Slider.h"
#include "PerlinNoise.h"

class PatchCableSource;

class FubbleModule : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   FubbleModule();
   ~FubbleModule();
   static IDrawableModule* Create() { return new FubbleModule(); }


   void CreateUIControls() override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   float GetPlaybackTime(double time);
   ofRectangle GetFubbleRect();
   ofVec2f GetFubbleMouseCoord();
   void RecordPoint();
   bool IsHovered();
   void Clear();
   float GetPerlinNoiseValue(double time, float x, float y, bool horizontal);
   void UpdatePerlinSeed() { mPerlinSeed = gRandom() % 1000; }

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   struct FubbleAxis : public IModulator
   {
      FubbleAxis(FubbleModule* owner, bool horizontal)
      : mOwner(owner)
      , mIsHorizontal(horizontal)
      , mHasRecorded(false)
      {
      }
      void UpdateControl() { OnModulatorRepatch(); }
      void SetCableSource(PatchCableSource* cableSource) { mTargetCable = cableSource; }
      PatchCableSource* GetCableSource() const { return mTargetCable; }

      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->Enabled() && (mHasRecorded || mOwner->mIsRightClicking); }

      FubbleModule* mOwner;
      bool mIsHorizontal;
      Curve mCurve;
      bool mHasRecorded;
   };

   FubbleAxis mAxisH;
   FubbleAxis mAxisV;
   float mLength;
   bool mQuantizeLength;
   Checkbox* mQuantizeLengthCheckbox;
   NoteInterval mQuantizeInterval;
   DropdownList* mQuantizeLengthSelector;
   float mSpeed;
   FloatSlider* mSpeedSlider;
   ClickButton* mClearButton;
   float mWidth;
   float mHeight;
   double mRecordStartOffset;
   bool mIsDrawing;
   bool mIsRightClicking;
   float mMouseX;
   float mMouseY;
   PerlinNoise mNoise;
   float mPerlinStrength;
   FloatSlider* mPerlinStrengthSlider;
   float mPerlinScale;
   FloatSlider* mPerlinScaleSlider;
   float mPerlinSpeed;
   FloatSlider* mPerlinSpeedSlider;
   int mPerlinSeed;
   ClickButton* mUpdatePerlinSeedButton;
};
