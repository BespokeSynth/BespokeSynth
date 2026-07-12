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
//  BlobViz.h
//  Bespoke
//
//  Audio-reactive blob visualizer (pass-through tap, like Lissajous). Draws several soft glowing
//  "cells" that drift and overlap/merge, leave decaying trail echoes, and shift warm->cool with the
//  sound. An N-fold rotational symmetry option turns the whole scene into a kaleidoscope. Bespoke's
//  OF port has no shaders/FBOs, so glow is layered translucent circles and the feedback trail is a
//  CPU history buffer of recent frames drawn with decaying alpha.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include <vector>

class BlobViz : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   BlobViz();
   virtual ~BlobViz();
   static IDrawableModule* Create() { return new BlobViz(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = w;
      mHeight = h;
   }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void PaletteColor(float t, float& rOut, float& gOut, float& bOut) const;

   //audio analysis (written on the audio thread, read on the UI thread - plain floats only)
   float mAmplitude{ 0 };
   float mHighEnergy{ 0 };

   //controls
   float mSensitivity{ 1.0f };
   float mTrailDecay{ 0.92f };
   float mGlow{ 1.0f };
   float mGrain{ 0.05f };
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 0 };
   int mNumBlobs{ 3 };
   int mSymmetry{ 1 };

   FloatSlider* mSensitivitySlider{ nullptr };
   FloatSlider* mTrailSlider{ nullptr };
   FloatSlider* mGlowSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };
   IntSlider* mNumBlobsSlider{ nullptr };
   IntSlider* mSymmetrySlider{ nullptr };

   //CPU trail history: a ring buffer of recent FRAMES, each frame holding the several blobs drawn
   //that frame. Frames are drawn oldest->newest with decaying alpha.
   struct Blob
   {
      float x{ 0 }, y{ 0 };
      float radius{ 0 };
      float r{ 1 }, g{ 1 }, b{ 1 };
   };
   static constexpr int kMaxBlobs = 5;
   struct Frame
   {
      Blob blobs[kMaxBlobs];
      int count{ 0 };
   };
   static constexpr int kHistory = 72;
   std::vector<Frame> mHistory;
   int mHistoryPos{ 0 };
   int mHistoryCount{ 0 };
};
