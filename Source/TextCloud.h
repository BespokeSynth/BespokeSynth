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
//  TextCloud.h
//  Bespoke
//
//  Type any text and it becomes a 3D object floating in space: the text is rasterised (JUCE font)
//  into a pixel grid, each lit pixel is extruded into a short stack of points along Z to give the
//  letters real depth, and the whole slab tumbles in 3D (auto-spin + click-drag). Colour comes from
//  a cosine palette / mono, with a live hue control. Audio-reactive pass-through drives spin, depth
//  and jitter. Cheap immediate-mode effects layer on top: grain, blur, bloom, distortion. Point
//  count is hard-capped so the frame rate stays high - no shaders, no feedback buffer.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "IVisualNode.h"
#include "VizGL.h"
#include <string>
#include <vector>

class TextCloud : public IAudioProcessor, public IDrawableModule, public IVisualNode, public IFloatSliderListener, public IDropdownListener, public ITextEntryListener
{
public:
   TextCloud();
   virtual ~TextCloud();
   static IDrawableModule* Create() { return new TextCloud(); }
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
   void OnClicked(float x, float y, bool right) override;
   void Poll() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

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

   void RebuildVoxels();
   void PaletteColor(float t, float& rOut, float& gOut, float& bOut) const;

   struct Voxel
   {
      float u, v; //centred, normalised position (-0.5..0.5-ish)
      float lum; //0..1
   };
   std::vector<Voxel> mVoxels;
   char mText[MAX_TEXTENTRY_LENGTH]{}; //TextEntry copies using this fixed size - must match to avoid overflow
   std::string mLastBuilt;

   //audio analysis (audio thread writes, UI thread reads)
   float mAmplitude{ 0 };
   float mHighEnergy{ 0 };

   //controls
   float mExtrude{ 0.35f }; //Z depth of the extruded slab
   float mSpinSpeed{ 0.6f };
   float mSensitivity{ 1.0f };
   float mDistort{ 0.0f };
   float mBlur{ 0.0f };
   float mBloom{ 0.25f };
   float mGrain{ 0.04f };
   float mExposure{ 1.0f };
   float mHueShift{ 0.0f };
   int mColorMode{ 0 }; //0 palette, 1 mono
   int mPaletteIndex{ 0 };
   int mFontIndex{ 0 };
   int mLastFontIndex{ -1 };

   TextEntry* mTextEntry{ nullptr };
   DropdownList* mFontSelector{ nullptr };
   FloatSlider* mExtrudeSlider{ nullptr };
   FloatSlider* mSpinSlider{ nullptr };
   FloatSlider* mSensitivitySlider{ nullptr };
   FloatSlider* mDistortSlider{ nullptr };
   FloatSlider* mBlurSlider{ nullptr };
   FloatSlider* mBloomSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mColorModeSelector{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   //3D tumble + drag
   float mRotX{ -0.2f };
   float mRotY{ 0.4f };
   float mRotZ{ 0.0f };
   bool mDragging{ false };
   float mLastDragX{ 0 };
   float mLastDragY{ 0 };

   int mAspectRows{ 1 }; //grid rows used at build time (for aspect)
   int mAspectCols{ 1 };

   float mWidth{ 380 };
   float mHeight{ 338 };

   VizGL::Fbo mOutputFbo;
   int mLastCookFrame{ -1 };
};
