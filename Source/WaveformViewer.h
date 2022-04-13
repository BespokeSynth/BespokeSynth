/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
//  WaveformViewer.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/19/12.
//
//

#ifndef __modularSynth__WaveformViewer__
#define __modularSynth__WaveformViewer__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "TextEntry.h"
#include "INoteReceiver.h"

#define BUFFER_VIZ_SIZE 10000

class WaveformViewer : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public INoteReceiver
{
public:
   WaveformViewer();
   virtual ~WaveformViewer();
   static IDrawableModule* Create() { return new WaveformViewer(); }


   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   void TextEntryComplete(TextEntry* entry) override {}

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   float mAudioView[BUFFER_VIZ_SIZE][2];
   bool mDoubleBufferFlip;

   int mBufferVizOffset[2];
   float mVizPhase[2];

   float mDisplayFreq;
   int mLengthSamples;
   float mDrawGain;
   bool mPhaseAlign;
   float mWidth;
   float mHeight;
   bool mDrawWaveform;
   bool mDrawCircle;

   FloatSlider* mHueNote;
   FloatSlider* mHueAudio;
   FloatSlider* mHueInstrument;
   FloatSlider* mHueNoteSource;
   FloatSlider* mSaturation;
   FloatSlider* mBrightness;
   TextEntry* mDisplayFreqEntry;
   IntSlider* mLengthSamplesSlider;
   FloatSlider* mDrawGainSlider;
};

#endif /* defined(__modularSynth__WaveformViewer__) */
