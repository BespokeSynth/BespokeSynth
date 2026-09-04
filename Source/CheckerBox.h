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
//  CheckerBox.h
//  Bespoke
//
//  Optical-illusion checkerboard visualizer. Same module as before (grid/bend/spiral/tunnel patterns,
//  colour flip, chroma aberration, grain, black-white or palette colours, audio-reactive) but rendered
//  on the GPU via a fragment shader instead of CPU quads. Also exposes its output as an IVisualNode so
//  it plugs into the visual graph (Composite / VizFilter).
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IVisualNode.h"
#include "VizGL.h"

class CheckerBox : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IVisualNode
{
public:
   CheckerBox();
   virtual ~CheckerBox();
   static IDrawableModule* Create() { return new CheckerBox(); }
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

   //IVisualNode
   unsigned int GetOutputTexture() override { return VizGL::FboTexture(mOut); }
   int GetOutputWidth() const override { return mOut.w; }
   int GetOutputHeight() const override { return mOut.h; }
   void CookIfNeeded(int frameId) override { Cook(); }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   enum Pattern
   {
      kPattern_Grid = 0,
      kPattern_Bend,
      kPattern_Spiral,
      kPattern_Tunnel
   };

   bool EnsureShader();
   void Cook();

   //audio analysis
   float mAmplitude{ 0 };

   //controls (unchanged from the original)
   float mSpeed{ 4.0f };
   int mGridN{ 10 };
   float mDistort{ 0.0f };
   int mPattern{ kPattern_Grid };
   float mChroma{ 0.25f };
   float mGrain{ 0.04f };
   float mExposure{ 1.0f };
   float mSensitivity{ 1.0f };
   int mColorMode{ 0 };
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 0 };

   FloatSlider* mSpeedSlider{ nullptr };
   IntSlider* mGridSlider{ nullptr };
   FloatSlider* mDistortSlider{ nullptr };
   DropdownList* mPatternSelector{ nullptr };
   FloatSlider* mChromaSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mSensitivitySlider{ nullptr };
   DropdownList* mColorModeSelector{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   //gpu
   VizGL::Fbo mOut;
   unsigned int mProgram{ 0 };
   bool mShaderTried{ false };
   int mResW{ 256 };
   int mResH{ 256 };

   int mLocTime{ -1 }, mLocStep{ -1 }, mLocReact{ -1 }, mLocDistort{ -1 }, mLocPattern{ -1 };
   int mLocChroma{ -1 }, mLocGrain{ -1 }, mLocExposure{ -1 }, mLocColorMode{ -1 }, mLocGrid{ -1 }, mLocRes{ -1 };
   int mLocPalA{ -1 }, mLocPalB{ -1 }, mLocPalC{ -1 }, mLocPalD{ -1 }, mLocHue{ -1 };

   float mWidth{ 360 };
   float mHeight{ 300 };
};
