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
//  CubeViz.h
//  Bespoke
//
//  Audio-reactive visualizer (pass-through tap): a stack of 1-10 tumbling 3D shapes that spin,
//  resize and pull apart with the audio. The shape is chosen from a library of parametric surfaces
//  (cube, sphere, cone, cylinder, torus, ellipsoid, hyperboloid, paraboloid + a set of Paul-Bourke
//  surfaces). A symmetry toggle mirrors the stack for a kaleidoscopic look. Tiny CPU 3D pipeline:
//  sample the surface on a grid, rotate on all three axes, painter-sort the quads, flat light-shade
//  each and colour by a cosine palette, grain on top. NO trail/feedback buffer -> high frame rate.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include <vector>

class CubeViz : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener
{
public:
   CubeViz();
   virtual ~CubeViz();
   static IDrawableModule* Create() { return new CubeViz(); }
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
   void ButtonClicked(ClickButton* button, double time) override;

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
   void SurfacePoint(int shape, float u01, float v01, float& x, float& y, float& z) const;
   void DrawShape(float cx, float cy, float halfW, float halfH, float rx, float ry, float rz, int shape, float paletteT, int gridN, bool mirror);

   //audio analysis (audio thread writes, UI thread reads)
   float mAmplitude{ 0 };
   float mHighEnergy{ 0 };

   //controls
   float mSensitivity{ 1.0f };
   float mSpinSpeed{ 1.0f };
   float mSeparation{ 1.0f };
   int mShape{ 0 };
   int mCountX{ 3 }; //columns (horizontal)
   int mCountY{ 1 }; //rows (vertical)
   bool mSymmetry{ false };
   float mGrain{ 0.06f };
   float mExposure{ 1.0f };
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 0 };

   //randomize: each grid cell can get its own random shape
   static const int kMaxCells = 100;
   int mShapeGrid[kMaxCells];
   bool mRandomized{ false };

   //free-tumbling rotation accumulators (all three axes -> 360 in space)
   float mRotX{ 0.0f };
   float mRotY{ 0.0f };
   float mRotZ{ 0.0f };

   FloatSlider* mSensitivitySlider{ nullptr };
   FloatSlider* mSpinSlider{ nullptr };
   FloatSlider* mSeparationSlider{ nullptr };
   DropdownList* mShapeSelector{ nullptr };
   IntSlider* mCountXSlider{ nullptr };
   IntSlider* mCountYSlider{ nullptr };
   ClickButton* mRandomizeButton{ nullptr };
   ClickButton* mResetButton{ nullptr };
   Checkbox* mSymmetryCheckbox{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   float mWidth{ 320 };
   float mHeight{ 398 };
};
