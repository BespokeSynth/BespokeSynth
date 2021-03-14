/*
  ==============================================================================

    LooperGranulator.h
    Created: 13 Mar 2021 1:55:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "FloatSliderLFOControl.h"
#include "Slider.h"
#include "Ramp.h"
#include "DropdownList.h"
#include "Granulator.h"

class Looper;

class LooperGranulator : public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   LooperGranulator();
   virtual ~LooperGranulator();
   static IDrawableModule* Create() { return new LooperGranulator(); }

   string GetTitleLabel() override { return "looper granulator"; }
   void CreateUIControls() override;

   void ProcessFrame(double time, float bufferOffset, float* output);
   void DrawOverlay(ofRectangle bufferRect, int loopLength);
   bool IsActive() { return mOn; }
   bool ShouldFreeze() { return mOn && mFreeze; }
   void OnCommit();

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;

   float mWidth;
   float mHeight;
   PatchCableSource* mLooperCable;
   Looper* mLooper;
   bool mOn;
   Checkbox* mOnCheckbox;
   Granulator mGranulator;
   FloatSlider* mGranOverlap;
   FloatSlider* mGranSpeed;
   FloatSlider* mGranLengthMs;
   float mDummyPos;
   FloatSlider* mPosSlider;
   bool mFreeze;
   Checkbox* mFreezeCheckbox;
   FloatSlider* mGranPosRandomize;
   FloatSlider* mGranSpeedRandomize;
   Checkbox* mGranOctaveCheckbox;
};