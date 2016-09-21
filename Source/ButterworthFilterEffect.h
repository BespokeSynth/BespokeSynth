//
//  ButterworthFilterEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__ButterworthFilterEffect__
#define __Bespoke__ButterworthFilterEffect__

#include "IAudioProcessor.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "FilterButterworth24db.h"

class ButterworthFilterEffect : public IAudioProcessor, public IDropdownListener, public IFloatSliderListener
{
public:
   ButterworthFilterEffect();
   ~ButterworthFilterEffect();
   
   static IAudioProcessor* Create() { return new ButterworthFilterEffect(); }
   
   string GetTitleLabel() override { return "butterworth"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetFilterParams(float f, float q) { mButterworth.Set(f, q); }
   
   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
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
   void GetModuleDimensions(int& width, int& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   void ResetFilter();
   
   float mF;
   FloatSlider* mFSlider;
   float mQ;
   FloatSlider* mQSlider;
   
   CFilterButterworth24db mButterworth;
   float* mDryBuffer;
   int mDryBufferSize;
   
   bool mCoefficientsHaveChanged;
};

#endif /* defined(__Bespoke__ButterworthFilterEffect__) */
