//
//  FreeverbOutput.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/14.
//
//

#ifndef __Bespoke__FreeverbOutput__
#define __Bespoke__FreeverbOutput__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"
#include "freeverb/revmodel.hpp"
#include "Ramp.h"

class FreeverbOutput;

extern FreeverbOutput* TheFreeverbOutput;

class FreeverbOutput : public IDrawableModule, public IFloatSliderListener
{
public:
   FreeverbOutput();
   ~FreeverbOutput();
   static IDrawableModule* Create() { return new FreeverbOutput(); }
   static bool CanCreate() { return TheFreeverbOutput == NULL; }
   
   string GetTitleLabel() override { return "freeverb output"; }
   void CreateUIControls() override;
   
   void ProcessAudio(float* left, float* right, int bufferSize);
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   int GetLeftChannel() const { return mLeftChannel; }
   int GetRightChannel() const { return mRightChannel; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int& y) override;
   bool Enabled() const override { return mEnabled; }
   
   revmodel mFreeverb;
   bool mNeedUpdate;
   bool mFreeze;
   float mRoomSize;
   float mDamp;
   float mWet;
   float mDry;
   float mVerbWidth;
   int mLeftChannel;
   int mRightChannel;
   FloatSlider* mRoomSizeSlider;
   FloatSlider* mDampSlider;
   FloatSlider* mWetSlider;
   FloatSlider* mDrySlider;
   FloatSlider* mWidthSlider;
};

#endif /* defined(__Bespoke__FreeverbOutput__) */
