//
//  Stereofier.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/18/12.
//
//

#ifndef __modularSynth__Stereofier__
#define __modularSynth__Stereofier__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "Ramp.h"
#include "Checkbox.h"
#include "PatchCableSource.h"

class Stereofier : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IButtonListener, public IIntSliderListener
{
public:
   Stereofier();
   virtual ~Stereofier();
   static IDrawableModule* Create() { return new Stereofier(); }
   
   string GetTitleLabel() override { return "stereo"; }
   void CreateUIControls() override;

   void SetPan(float pan) { mPan = pan; }
   IAudioReceiver* GetTarget2() override { return mPatchCableSource2 ? mPatchCableSource2->GetAudioReceiver() : NULL; }
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=120; h=40; }
   bool Enabled() const override { return mEnabled; }
   
   int mInputBufferSize;
   float* mInputBuffer;
   float* mInputBuffer2;
   RollingBuffer mVizBuffer2;
   PatchCableSource* mPatchCableSource2;
   float mPan;
   Ramp mPanRamp;
   FloatSlider* mPanSlider;
   float mWiden;
   FloatSlider* mWidenSlider;
   RollingBuffer mWidenerBuffer;
};

#endif /* defined(__modularSynth__Stereofier__) */

