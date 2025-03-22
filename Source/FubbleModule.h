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
#include "IModulator.h"
#include "Slider.h"
#include "PerlinNoise.h"

namespace
{
   const int kTopControlHeight = 22;
   const int kTimelineSectionHeight = 50;
   const int kBottomControlHeight = 58;
}

class PatchCableSource;

class FubbleModule : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   FubbleModule();
   ~FubbleModule();
   static IDrawableModule* Create() { return new FubbleModule(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

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
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   struct FubbleAxis : public IModulator
   {
      FubbleAxis(FubbleModule* owner, bool horizontal)
      : mOwner(owner)
      , mIsHorizontal(horizontal)
      {
      }
      void UpdateControl() { OnModulatorRepatch(); }
      void SetCableSource(PatchCableSource* cableSource) { mTargetCable = cableSource; }
      PatchCableSource* GetCableSource() const { return mTargetCable; }

      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->IsEnabled() && (mHasRecorded || mOwner->mIsRightClicking); }

      FubbleModule* mOwner{ nullptr };
      bool mIsHorizontal{ false };
      Curve mCurve{ 0 };
      bool mHasRecorded{ false };
   };

   FubbleAxis mAxisH;
   FubbleAxis mAxisV;
   float mLength{ 0 };
   bool mQuantizeLength{ false };
   Checkbox* mQuantizeLengthCheckbox{ nullptr };
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_4n };
   DropdownList* mQuantizeLengthSelector{ nullptr };
   float mSpeed{ 1 };
   FloatSlider* mSpeedSlider{ nullptr };
   ClickButton* mClearButton{ nullptr };
   float mWidth{ 220 };
   float mHeight{ kTopControlHeight + 200 + kTimelineSectionHeight + kBottomControlHeight };
   double mRecordStartOffset{ 0 };
   bool mIsDrawing{ false };
   bool mIsRightClicking{ false };
   float mMouseX{ 0 };
   float mMouseY{ 0 };
   PerlinNoise mNoise;
   float mPerlinStrength{ 0 };
   FloatSlider* mPerlinStrengthSlider{ nullptr };
   float mPerlinScale{ 1 };
   FloatSlider* mPerlinScaleSlider{ nullptr };
   float mPerlinSpeed{ 1 };
   FloatSlider* mPerlinSpeedSlider{ nullptr };
   int mPerlinSeed{ 0 };
   ClickButton* mUpdatePerlinSeedButton{ nullptr };
};
