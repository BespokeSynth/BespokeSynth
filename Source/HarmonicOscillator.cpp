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
/*
  ==============================================================================

    HarmonicOscillator.cpp
    Created: Nov 2025
    Author:  Mat Warren

  ==============================================================================
*/


// This file implements the actual behaviour of the HarmonicOscillator module.
// It handles:
//  - Creating jacks and sliders
//  - Drawing the UI and little waveform preview
//  - Responding to MIDI note messages
//  - Taking in CV pitch modulation
//  - Generating the audio in the Process() method

#include "HarmonicOscillator.h"
#include "IAudioReceiver.h"
#include "SynthGlobals.h" // contains global sample rate + other DSP globals
#include "ModularSynth.h" // gives access to modular system structure
#include "Profiler.h" // small profiling macro
#include "OpenFrameworksPort.h" // used for drawing items (text, shapes)
#include <cmath>


//  ------------------ CONSTRUCTOR----------
// 
// Gets called when the module is first created.
HarmonicOscillator::HarmonicOscillator()
{
   SetName("harmonic osc"); // Name that appears in the module title bar

   // --- AUDIO OUTPUT JACK ---
   // This lets the module send audio into other modules.
   // All audio you generate in Process() ends up here.
   mAudioOut = new PatchCableSource(this, ConnectionType::kConnectionType_Audio);
   AddPatchCableSource(mAudioOut);

   // --- PITCH CV INPUT JACK ---
   // Sequencers / LFOs plug into this.
   // It sits visually on the left side of the module.
   mFreqInput = new PatchCableSource(this, ConnectionType::kConnectionType_Audio);
   AddPatchCableSource(mFreqInput);
   mFreqInput->SetManualSide(PatchCableSource::Side::kLeft);
}


//  ------------UI CREATION----------

// Called when the module is created.
// This is where sliders/buttons are constructed and positioned.
void HarmonicOscillator::CreateUIControls()
{
   // Draws the base panel graphics
   IDrawableModule::CreateUIControls();

   // Main base pitch control
   mFreqSlider = new FloatSlider(this, "freq",
                                 5, 2, 140, 15,
                                 &mBaseFreq, 20.f, 2000.f);

   const int baseY = 35;

   // --- HARMONIC GAIN SLIDERS (volume of each harmonic) ---
   for (int i = 0; i < 8; ++i)
   {
      mHarmonicSliders[i] = new FloatSlider(
      this,
      ("h" + std::to_string(i + 1)).c_str(), // labels: h1, h2, etc.
      5,
      baseY + i * 22,
      60,
      18,
      &mHarmonicGains[i],
      0.f,
      1.f,
      2);
   }

   // --- HARMONIC RATIO SLIDERS (frequency multipliers) ---
   // These are integers (snapped later)
   for (int i = 0; i < 8; ++i)
   {
      mHarmonicRatioSliders[i] = new FloatSlider(
      this,
      "",
      70,
      baseY + i * 22,
      60,
      18,
      &mHarmonicRatios[i],
      1.0f,
      16.0f,
      0 // no decimals shown
      );
   }

   // Resize module vertically to fit everything nicely
   mHeight = baseY + (8 * 22) + 40;
}


//  ------------DRAW UI (normal clipped drawing region)----------

void HarmonicOscillator::DrawModule()
{
   if (Minimized() || !IsVisible())
      return;

   // Titles for the two slider columns
   ofPushStyle();
   ofSetColor(255, 255, 255, 180);
   DrawTextNormal("Gain", 5, 30);
   DrawTextNormal("Ratio", 70, 30);
   ofPopStyle();

   // Draw the base frequency slider
   mFreqSlider->Draw();

   // Draw harmonic gain sliders
   for (int i = 0; i < 8; ++i)
      mHarmonicSliders[i]->Draw();

   // Draw harmonic ratio sliders
   for (int i = 0; i < 8; ++i)
      mHarmonicRatioSliders[i]->Draw();
}

//  -------------DRAW UI (unclipped region) — waveform preview----------

void HarmonicOscillator::DrawModuleUnclipped()
{
   if (Minimized() || !IsVisible())
      return;

   // This draws a simple preview of the resulting waveform
   // so you have immediate visual feedback.

   const float margin = 5.0f;
   const float oscHeight = 30.0f;

   float x = margin;
   float y = mHeight - oscHeight - margin;
   float w = mWidth - margin * 2.0f;
   float h = oscHeight;

   ofPushStyle();

   // Background
   ofSetColor(0, 0, 0, 80);
   ofFill();
   ofRect(x, y, w, h);

   // Border
   ofNoFill();
   ofSetColor(100, 100, 100, 180);
   ofRect(x, y, w, h);

   // Waveform line
   ofSetColor(245, 58, 0, 200);
   ofNoFill();
   ofBeginShape();

   int numPoints = static_cast<int>(w);
   if (numPoints < 2)
      numPoints = 2;

   // One full cycle of the oscillator
   for (int i = 0; i < numPoints; ++i)
   {
      float t = (float)i / (float)(numPoints - 1);
      float phase = t * FTWO_PI;

      float sample = 0.0f;

      // Sum each harmonic using its ratio & gain
      for (int hIndex = 0; hIndex < 8; ++hIndex)
      {
         float ratio = std::round(mHarmonicRatios[hIndex]);
         float gain = mHarmonicGains[hIndex];
         sample += std::sin(phase * ratio) * gain;
      }

      sample = ofClamp(sample, -1.0f, 1.0f);

      float px = x + i;
      float py = y + ofMap(sample, -1.0f, 1.0f, h, 0.0f, true);

      ofVertex(px, py);
   }

   ofEndShape(false);
   ofPopStyle();
}

//  -------------------MIDI NOTE HANDLING------------------------
// 
// This is called by Bespoke whenever a MIDI note message hits the module.
// It updates mNoteFreq so the oscillator knows what pitch to play.
void HarmonicOscillator::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
      mNoteFreq = 440.f * powf(2.f, (note.pitch - 69) / 12.f);
   else
      mNoteFreq = 0.f; // release note → return to knob pitch
}

//  ----------------CV PITCH HELPER----------------------------
// 
// Converts CV input (-1..+1) into pitch shift in semitones.
static float ApplyPitchCV(float baseFreq, PatchCableSource* cvInput)
{
   if (!cvInput || !cvInput->GetTarget())
      return baseFreq;

   IAudioReceiver* recv = cvInput->GetAudioReceiver();
   if (!recv)
      return baseFreq;

   ChannelBuffer* buf = recv->GetBuffer();
   float* cv = buf->GetChannel(0);
   int bufferSize = buf->BufferSize();

   // Average CV instead of sample-by-sample (more stable)
   float sum = 0.f;
   for (int i = 0; i < bufferSize; ++i)
      sum += cv[i];

   float avg = sum / bufferSize;

   // Map -1..1 to -12..+12 semitones
   float semitones = avg * 12.0f;

   return baseFreq * powf(2.0f, semitones / 12.0f);
}


//  -------------AUDIO DSP — MAIN OSCILLATOR SECTION----------

void HarmonicOscillator::Process(double time)
{
   PROFILER(HarmonicOscillator);

   // Get the module we are sending audio to
   IAudioReceiver* target = GetTarget();
   if (!target)
      return;

   ChannelBuffer* out = target->GetBuffer();
   float* outCh = out->GetChannel(0);
   int bufferSize = out->BufferSize();
   const float sampleRate = gSampleRate;

   // Note pitch overrides slider pitch
   float freq = (mNoteFreq > 0.f ? mNoteFreq : mBaseFreq);

   // Add CV pitch modulation
   freq = ApplyPitchCV(freq, mFreqInput);

   mFrequency = freq; // store for UI etc.

   // Prevent clipping by normalising the total gain
   float gainSum = 0.f;
   for (int h = 0; h < 8; ++h)
      gainSum += fabs(mHarmonicGains[h]);

   float norm = (gainSum > 1.f ? 1.f / gainSum : 1.f);

   // --- MAIN DSP LOOP ---
   for (int i = 0; i < bufferSize; ++i)
   {
      float sample = 0.f;

      // Add all harmonics
      for (int hIndex = 0; hIndex < 8; ++hIndex)
      {
         float ratio = mHarmonicRatios[hIndex];
         float localPhase = mPhase * ratio;
         sample += std::sin(localPhase) * mHarmonicGains[hIndex];
      }

      sample *= norm;
      outCh[i] = sample;

      // Increment the oscillator phase (wrap around)
      mPhase += (mFrequency / sampleRate) * TWO_PI;
      if (mPhase > TWO_PI)
         mPhase -= TWO_PI;
   }

   // Send audio to internal oscilloscope visualiser
   GetVizBuffer()->WriteChunk(outCh, bufferSize, 0);
}


//  ---------SLIDER UPDATE HANDLER----------

// Called whenever a slider changes. For ratio sliders, force integer values

void HarmonicOscillator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   for (int i = 0; i < 8; ++i)
   {
      if (slider == mHarmonicRatioSliders[i])
      {
         float snapped = std::round(mHarmonicRatios[i]);
         mHarmonicRatios[i] = ofClamp(snapped, 1.0f, 16.0f);
         return;
      }
   }
}
