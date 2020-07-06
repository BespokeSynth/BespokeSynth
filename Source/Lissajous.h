//
//  Lissajous.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#ifndef __Bespoke__Lissajous__
#define __Bespoke__Lissajous__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"

#define NUM_LISSAJOUS_POINTS 3000

class Lissajous : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   Lissajous();
   virtual ~Lissajous();
   static IDrawableModule* Create() { return new Lissajous(); }
   
   string GetTitleLabel() override { return "lissajous"; }
   void CreateUIControls() override;
   
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   float mScale;
   FloatSlider* mScaleSlider;
   
   ofVec2f mLissajousPoints[NUM_LISSAJOUS_POINTS];
   int mOffset;
   bool mAutocorrelationMode;
   bool mOnlyHasOneChannel;
};


#endif /* defined(__Bespoke__Lissajous__) */
