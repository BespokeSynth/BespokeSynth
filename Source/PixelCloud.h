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
//  PixelCloud.h
//  Bespoke
//
//  Drag & drop any image; it is downsampled to a grid and rendered two ways:
//    - ascii : a glitchy ASCII/halftone block field (per-lum glyphs, RGB channel-split, row-tear)
//    - cloud : every cell becomes a point in a 3D cloud you can spin (auto + click-drag) and that
//              extrudes on the Z axis by brightness
//  Audio-reactive pass-through tap drives the glitch / jitter / spin. Colour comes from the image,
//  a cosine palette, or mono. Everything is CPU immediate-mode with a hard cap on the cell count,
//  so the frame rate stays high - no feedback buffer, no per-frame allocations, no shaders.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include <string>
#include <vector>

class PixelCloud : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   PixelCloud();
   virtual ~PixelCloud();
   static IDrawableModule* Create() { return new PixelCloud(); }
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

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void OnClicked(float x, float y, bool right) override; //start a rotate-drag (cloud mode)
   void Poll() override; //continue the rotate-drag

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
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

   bool LoadImageFile(const std::string& path);
   void RebuildGrid();
   void PaletteColor(float t, float& rOut, float& gOut, float& bOut) const;
   void CellColor(int idx, float lum, float& rOut, float& gOut, float& bOut) const;

   enum Mode
   {
      kMode_Ascii = 0,
      kMode_Cloud
   };

   //source image kept at full res, grid rebuilt from it on demand
   std::vector<unsigned char> mImgRGB; //mImgW*mImgH*3
   int mImgW{ 0 };
   int mImgH{ 0 };
   std::string mImagePath;

   //downsampled grid (rebuilt only when the image or density changes)
   int mCols{ 0 };
   int mRows{ 0 };
   std::vector<float> mLum; //cols*rows
   std::vector<float> mR, mG, mB; //cols*rows, 0..1

   //audio analysis (audio thread writes, UI thread reads)
   float mAmplitude{ 0 };
   float mHighEnergy{ 0 };

   //controls
   int mMode{ kMode_Ascii };
   int mDensity{ 70 }; //grid width in cells
   float mGlitch{ 0.35f };
   float mDepth{ 1.0f }; //z extrusion (cloud) / channel-split (ascii)
   float mSpinSpeed{ 0.6f };
   float mSensitivity{ 1.0f };
   float mGrain{ 0.05f };
   float mExposure{ 1.0f };
   float mHueShift{ 0.0f };
   int mColorMode{ 0 }; //0 image, 1 palette, 2 mono
   int mPaletteIndex{ 0 };

   DropdownList* mModeSelector{ nullptr };
   IntSlider* mDensitySlider{ nullptr };
   FloatSlider* mGlitchSlider{ nullptr };
   FloatSlider* mDepthSlider{ nullptr };
   FloatSlider* mSpinSlider{ nullptr };
   FloatSlider* mSensitivitySlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mColorModeSelector{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   //3D tumble
   float mRotX{ -0.3f };
   float mRotY{ 0.5f };
   float mRotZ{ 0.0f };

   //rotate-drag state
   bool mDragging{ false };
   float mLastDragX{ 0 };
   float mLastDragY{ 0 };

   float mWidth{ 360 };
   float mHeight{ 348 };
};
