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

#pragma once

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
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void TextEntryComplete(TextEntry* entry) override {}

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mAudioView[BUFFER_VIZ_SIZE][2]{};
   bool mDoubleBufferFlip{ false };

   int mBufferVizOffset[2]{};
   float mVizPhase[2]{};

   float mDisplayFreq{ 220 };
   int mLengthSamples{ 2048 };
   float mDrawGain{ 2 };
   bool mPhaseAlign{ true };
   float mWidth{ 600 };
   float mHeight{ 150 };
   bool mDrawWaveform{ true };
   bool mDrawCircle{ false };

   FloatSlider* mHueNote{ nullptr };
   FloatSlider* mHueAudio{ nullptr };
   FloatSlider* mHueInstrument{ nullptr };
   FloatSlider* mHueNoteSource{ nullptr };
   FloatSlider* mSaturation{ nullptr };
   FloatSlider* mBrightness{ nullptr };
   TextEntry* mDisplayFreqEntry{ nullptr };
   IntSlider* mLengthSamplesSlider{ nullptr };
   FloatSlider* mDrawGainSlider{ nullptr };
};
