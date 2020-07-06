//
//  ADSRDisplay.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/28/13.
//
//

#ifndef __modularSynth__ADSRDisplay__
#define __modularSynth__ADSRDisplay__

#include <iostream>
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
   void SetMaxTime(float maxTime);
   void SetADSR(::ADSR* adsr);
   ::ADSR* GetADSR() { return mAdsr; }
   void SetActive(bool active);
   void SpawnEnvelopeEditor();
   
   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   bool CanBeTargetedBy(PatchCableSource* source) const override { return false; }
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
   enum DisplayMode
   {
      kDisplayEnvelope,
      kDisplaySliders
   };
   static void ToggleDisplayMode();
   
protected:
   ~ADSRDisplay();   //protected so that it can't be created on the stack

private:
   enum AdjustParam
   {
      kAdjustAttack,
      kAdjustDecaySustain,
      kAdjustRelease,
      kAdjustEnvelopeEditor,
      kAdjustNone
   } mAdjustMode;

   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   
   void UpdateSliderVisibility();
   
   float mWidth;
   float mHeight;
   float mVol;
   float mMaxTime;
   bool mClick;
   ::ADSR* mAdsr;
   ::ADSR mViewAdsr;   //for ADSR simulation in drawing
   ofVec2f mClickStart;
   ::ADSR mClickAdsr;
   bool mHighlighted;
   bool mActive;
   FloatSlider* mASlider;
   FloatSlider* mDSlider;
   FloatSlider* mSSlider;
   FloatSlider* mRSlider;
   static DisplayMode sDisplayMode;
   EnvelopeEditor* mEditor;
};


#endif /* defined(__modularSynth__ADSRDisplay__) */
