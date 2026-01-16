#pragma once

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

    HarmonicOscillator.h
    Created: Nov 2025
    Author:  Mat Warren

  ==============================================================================
*/

// This header file defines the class structure of the module.
// The .cpp implements everything, but this file explains
// what the module IS and how it plugs into the Bespoke system.

#include "IAudioSource.h" // lets the module *output* audio
#include "IAudioReceiver.h" // lets the module *receive* audio (CV)
#include "IDrawableModule.h" // drawing + layout interface
#include "INoteReceiver.h" // lets the module react to MIDI notes
#include "Slider.h" // UI component
#include "PatchCableSource.h" // jacks
#include "ofxJSONElement.h" // config save/load

// ===========================================================
//  MODULE CLASS DEFINITION
// ===========================================================
// This class is a full Bespoke module.
// It inherits from several "interfaces":
//    IAudioSource     → provides audio output
//    IDrawableModule  → draws UI, sliders, jacks, labels
//    INoteReceiver    → receives MIDI note messages
//    IFloatSliderListener → reacts when sliders change
//
// Bespoke’s modular system depends heavily on these interfaces.
class HarmonicOscillator
: public IAudioSource,
  public IDrawableModule,
  public INoteReceiver,
  public IFloatSliderListener
{
public:
   // -------------------------------------------------------
   // Constructor + destructor
   // -------------------------------------------------------
   HarmonicOscillator(); // Sets name + makes jacks
   ~HarmonicOscillator() {}

   // Factory method (needed by Bespoke to spawn the module)
   static IDrawableModule* Create() { return new HarmonicOscillator(); }

   // Module capabilities (shown in patching rules)
   static bool AcceptsAudio() { return false; } // no audio inputs
   static bool AcceptsNotes() { return true; } // accepts MIDI notes
   static bool AcceptsPulses() { return false; } // no triggers

   // -------------------------------------------------------
   // UI + DSP overrides from IDrawableModule / IAudioSource
   // -------------------------------------------------------
   void CreateUIControls() override; // build sliders
   void DrawModule() override; // draw UI
   void DrawModuleUnclipped() override; // draw waveform preview
   void Process(double time) override; // audio DSP runs every buffer

   // -------------------------------------------------------
   // MIDI handling (from INoteReceiver)
   // -------------------------------------------------------
   void PlayNote(NoteMessage note) override;
   void SendCC(int, int, int = -1) override {}

   // -------------------------------------------------------
   // Slider callback (from IFloatSliderListener)
   // -------------------------------------------------------
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   // Module size
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

private:
   // Layout
   float mWidth = 180;
   float mHeight = 200;

   // Patch cables (one output, one pitch CV input)
   PatchCableSource* mAudioOut = nullptr;
   PatchCableSource* mFreqInput = nullptr;

   // Main pitch slider
   FloatSlider* mFreqSlider = nullptr;

   // Harmonic amplitude sliders
   FloatSlider* mHarmonicSliders[8]{};
   float mHarmonicGains[8] = { 1.f, 0.5f, 0.3f, 0.2f, 0.f, 0.f, 0.f, 0.f };

   // Base pitch state
   float mBaseFreq = 110.f;
   float mNoteFreq = 0.f;
   float mFrequency = 110.f;

   // Harmonic ratio sliders
   FloatSlider* mHarmonicRatioSliders[8]{};
   float mHarmonicRatios[8]{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f };

   // Base phase for oscillator
   float mPhase = 0.f;
};
