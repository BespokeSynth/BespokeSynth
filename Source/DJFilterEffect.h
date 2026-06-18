/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  DJFilterEffect.h
//
//  Created by Ryan Challinor on 5/29/26.
//
//

#pragma once

#include "IAudioEffect.h"
#include "Slider.h"
#include "BiquadFilter.h"
#include "IControlVisualizer.h"
#include "ClickButton.h"

class DJFilterEffect : public IAudioEffect, public IFloatSliderListener, public IControlVisualizer, public IButtonListener
{
public:
   DJFilterEffect();
   virtual ~DJFilterEffect();

   static IAudioEffect* Create() { return new DJFilterEffect(); }

   void CreateUIControls() override;

   void Init() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "djfilter"; }

   //IControlVisualizer
   void DrawVisualizationToScreen(AbletonMoveLCD* screen, IUIControl* control) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;

   float GetLowSignalLevel() const;
   float GetHighSignalLevel() const;
   float GetDrySignalLevel() const;
   float GetVolumeFadeLevel() const;
   float GetSignalResponseAt(float freq) const;

   FloatSlider* mTiltSlider{ nullptr };
   float mTilt{ 0.0f };
   FloatSlider* mQSlider{ nullptr };
   float mQ{ static_cast<float>(sqrt(2.0f) / 2) };

   ClickButton* mCenterButton{ nullptr };

   std::array<BiquadFilter, ChannelBuffer::kMaxNumChannels> mBiquadLow;
   std::array<BiquadFilter, ChannelBuffer::kMaxNumChannels> mBiquadHigh;
   ChannelBuffer mDryBuffer;
};
