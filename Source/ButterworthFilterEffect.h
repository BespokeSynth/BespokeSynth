//
//  ButterworthFilterEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__ButterworthFilterEffect__
#define __Bespoke__ButterworthFilterEffect__

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "FilterButterworth24db.h"

class ButterworthFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   ButterworthFilterEffect();
   ~ButterworthFilterEffect();
   
   static IAudioEffect* Create() { return new ButterworthFilterEffect(); }
   
   string GetTitleLabel() override { return "butterworth"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetFilterParams(float f, float q) { mButterworth[0].Set(f, q); }
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "butterworth"; }
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;
   
private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   void ResetFilter();
   
   float mF;
   FloatSlider* mFSlider;
   float mQ;
   FloatSlider* mQSlider;
   
   float mWidth;
   float mHeight;
   
   CFilterButterworth24db mButterworth[ChannelBuffer::kMaxNumChannels];
   ChannelBuffer mDryBuffer;
   
   bool mCoefficientsHaveChanged;
};

#endif /* defined(__Bespoke__ButterworthFilterEffect__) */
