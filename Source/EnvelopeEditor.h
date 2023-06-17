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

    EnvelopeEditor.h
    Created: 9 Nov 2017 5:20:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "ADSR.h"
#include "ADSRDisplay.h"

class EnvelopeControl
{
public:
   EnvelopeControl(ofVec2f position, ofVec2f dimensions);
   void SetADSR(::ADSR* adsr) { mAdsr = adsr; }
   void OnClicked(float x, float y, bool right);
   void MouseMoved(float x, float y);
   void MouseReleased();
   void Draw();
   void SetViewLength(float length) { mViewLength = length; }
   ofVec2f GetPosition() const { return mPosition; }
   ofVec2f GetDimensions() const { return mDimensions; }
   void SetPosition(ofVec2f pos) { mPosition = pos; }
   void SetDimensions(ofVec2f dim) { mDimensions = dim; }
   void SetFixedLengthMode(bool fixed) { mFixedLengthMode = fixed; }

private:
   void AddVertex(float x, float y);
   float GetPreSustainTime();
   float GetReleaseTime();
   float GetTimeForX(float x);
   float GetValueForY(float y);
   float GetXForTime(float time);
   float GetYForValue(float value);

   ofVec2f mPosition;
   ofVec2f mDimensions;
   ::ADSR* mAdsr{ nullptr };
   ::ADSR mClickAdsr;
   bool mClick{ false };
   ofVec2f mClickStart;
   float mViewLength{ 2000 };
   int mHighlightPoint{ -1 };
   int mHighlightCurve{ -1 };
   double mLastClickTime{ 0 };
   bool mFixedLengthMode{ false };
};

class EnvelopeEditor : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IIntSliderListener
{
public:
   EnvelopeEditor();
   static IDrawableModule* Create() { return new EnvelopeEditor(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }
   void Delete() { delete this; }
   void DrawModule() override;

   void SetEnabled(bool enabled) override {} //don't use this one
   bool IsEnabled() const override { return true; }
   bool HasTitleBar() const override { return mPinned; }
   bool IsSaveable() override { return mPinned; }
   void CreateUIControls() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   bool IsPinned() const { return mPinned; }
   void SetADSRDisplay(ADSRDisplay* adsrDisplay);
   bool HasSpecialDelete() const override { return true; }
   void DoSpecialDelete() override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

protected:
   ~EnvelopeEditor();

private:
   void OnClicked(float x, float y, bool right) override;
   void Pin();

   struct StageControls
   {
      FloatSlider* mTargetSlider{ nullptr };
      FloatSlider* mTimeSlider{ nullptr };
      FloatSlider* mCurveSlider{ nullptr };
      Checkbox* mSustainCheckbox{ nullptr };
      bool mIsSustainStage{ false };
   };

   EnvelopeControl mEnvelopeControl;

   float mWidth{ 320 };
   float mHeight{ 210 };

   ADSRDisplay* mADSRDisplay{ nullptr };
   ClickButton* mPinButton{ nullptr };
   bool mPinned{ false };
   FloatSlider* mADSRViewLengthSlider{ nullptr };
   FloatSlider* mMaxSustainSlider{ nullptr };
   Checkbox* mFreeReleaseLevelCheckbox{ nullptr };
   PatchCableSource* mTargetCable{ nullptr };
   std::array<StageControls, 10> mStageControls{};
};
