/*
  ==============================================================================

    ValueStream.h
    Created: 25 Oct 2020 7:09:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Transport.h"
#include "Slider.h"
#include <array>

class PatchCableSource;

class ValueStream : public IDrawableModule, public IAudioPoller, public IFloatSliderListener
{
public:
   ValueStream();
   ~ValueStream();
   static IDrawableModule* Create() { return new ValueStream(); }

   string GetTitleLabel() override { return "value stream"; }
   void CreateUIControls() override;

   IUIControl* GetUIControl() const { return mUIControl; }

   void OnTransportAdvanced(float amount) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;

   IUIControl* mUIControl;
   FloatSlider* mFloatSlider;
   PatchCableSource* mControlCable;
   float mWidth;
   float mHeight;
   float mSpeed;
   FloatSlider* mSpeedSlider;
   array<float, 100000> mValues;
   int mValueDisplayPointer;
};
