/*
  ==============================================================================

    GlobalControls.h
    Created: 26 Sep 2020 11:34:18am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Slider.h"

class GlobalControls : public IDrawableModule, public IFloatSliderListener
{
public:
   GlobalControls();
   virtual ~GlobalControls();
   static IDrawableModule* Create() { return new GlobalControls(); }

   string GetTitleLabel() override { return "global controls"; }
   void CreateUIControls() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;

   FloatSlider* mZoomSlider;
   FloatSlider* mXSlider;
   FloatSlider* mYSlider;
   FloatSlider* mMouseScrollXSlider;
   FloatSlider* mMouseScrollYSlider;
   FloatSlider* mBackgroundLissajousRSlider;
   FloatSlider* mBackgroundLissajousGSlider;
   FloatSlider* mBackgroundLissajousBSlider;

   float mWidth;
   float mHeight;
   float mMouseScrollX;
   float mMouseScrollY;
};