#pragma once

#include "IAudioEffect.h"
#include "Slider.h"

class LimiterEffect : public IAudioEffect, public IFloatSliderListener
{
public:
   LimiterEffect();

   static IAudioEffect* Create() { return new LimiterEffect(); }

   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   std::string GetType() override { return "limiter"; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   
   void SetRelease(float releaseMs);

   float mThreshold{ -0.1f };
   float mRelease{ 100.0f };
   float mInGain{ 0.0f };
   float mOutGain{ 0.0f };

   FloatSlider* mThresholdSlider{ nullptr };
   FloatSlider* mReleaseSlider{ nullptr };
   FloatSlider* mInGainSlider{ nullptr };
   FloatSlider* mOutGainSlider{ nullptr };

   double mReleaseCoef{ 0.0 };
   double mEnvelope{ 0.0 };
   double mGainReduction{ 1.0 }; // For UI
};
