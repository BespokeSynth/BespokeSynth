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
/*
  ==============================================================================

    AudioSyncer.h
    Created: 3 May 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Granulator.h"
#include "ADSR.h"

class Sample;

class AudioSyncer : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   AudioSyncer();
   ~AudioSyncer();
   static IDrawableModule* Create() { return new AudioSyncer(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   ChannelBuffer mWriteBuffer;
   Checkbox* mPassthroughCheckbox{ nullptr };
   bool mPassthrough{ false };
   FloatSlider* mDisplayLengthMsSlider{ nullptr };
   float mDisplayLengthMs{ 10000 };
   FloatSlider* mLatencyMsSlider{ nullptr };
   float mLatencyMs{ 0 };
   RollingBuffer mDelayBuffer;
   bool mMono{ true };
   BiquadFilter mBiquadFilter;
};
