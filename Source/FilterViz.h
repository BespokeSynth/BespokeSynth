//
//  FilterViz.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/24/14.
//
//

#ifndef __Bespoke__FilterViz__
#define __Bespoke__FilterViz__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Slider.h"
#include "ClickButton.h"
#include "IAudioEffect.h"

#define FILTER_VIZ_BINS 1024

class FilterViz : public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IButtonListener
{
public:
   FilterViz();
   ~FilterViz();
   static IDrawableModule* Create() { return new FilterViz(); }
   
   string GetTitleLabel() override { return "filter viz"; }
   void CreateUIControls() override;
   
   //IDrawableModule
   void Poll() override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
private:
   void GraphFilter();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=300; height=200; }
   
   float* mImpulseBuffer;
   float* mFFTOutReal;
   float* mFFTOutImag;
   
   bool mNeedUpdate;
   vector<IAudioEffect*> mFilters;
};

#endif /* defined(__Bespoke__FilterViz__) */
