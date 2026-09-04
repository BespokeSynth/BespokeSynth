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
//  ScopeViz.h
//  Bespoke
//
//  A real X/Y vector oscilloscope (the "oscilloscope music" look). It draws ONLY the incoming
//  signal - silence draws nothing, and the shape comes entirely from the audio.
//    - mono   : one input, plotted as X = signal, Y = 90 deg-shifted signal (Hilbert transform).
//               A single sine -> a perfect circle; harmonics -> Lissajous figures (bowties etc).
//    - stereo : X = left, Y = right (true scope; feed oscilloscope-music tracks or two oscillators).
//  Phosphor glow is faked with additive line passes (brighter where the beam moves slowly), over a
//  faint graticule, with palette/hue colouring and grain. CPU immediate-mode, no feedback buffer.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "IVisualNode.h"
#include "VizGL.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include <vector>

class ScopeViz : public IAudioProcessor, public IDrawableModule, public IVisualNode, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   ScopeViz();
   virtual ~ScopeViz();
   static IDrawableModule* Create() { return new ScopeViz(); }
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

   //IVisualNode
   void CookIfNeeded(int frameId) override;
   unsigned int GetOutputTexture() override;
   int GetOutputWidth() const override { return mOutputFbo.w; }
   int GetOutputHeight() const override { return mOutputFbo.h; }

   void GlowLine(float x0, float y0, float x1, float y1, float r, float g, float b, float bright, float glow);

   //rolling capture (audio thread writes, UI thread reads)
   static const int kScopeSize = 8192;
   std::vector<float> mScopeL; //mono input
   std::vector<float> mScopeQ; //90 deg-shifted mono (Hilbert output), aligned with a delayed input
   int mWritePos{ 0 };
   float mAmplitude{ 0 };

   //Hilbert FIR (odd length, antisymmetric) -> quadrature signal for the mono mode
   static const int kHilbertLen = 65;
   static const int kHilbertMid = 32; //(len-1)/2, the group delay to align X against
   std::vector<float> mHil;

   //controls
   float mGain{ 1.0f };
   float mGlow{ 0.7f };
   float mScaleAmt{ 0.9f };
   float mExposure{ 1.0f };
   int mDetail{ 900 };
   float mGrain{ 0.03f };
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 20 }; //matrix (green) by default - classic scope look
   bool mShowGrid{ true };

   FloatSlider* mGainSlider{ nullptr };
   FloatSlider* mGlowSlider{ nullptr };
   FloatSlider* mScaleSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   IntSlider* mDetailSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };
   Checkbox* mGridCheckbox{ nullptr };

   float mWidth{ 380 };
   float mHeight{ 320 };

   VizGL::Fbo mOutputFbo;
   int mLastCookFrame{ -1 };
};
