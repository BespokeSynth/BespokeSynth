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
//  An optical-illusion checkerboard visualizer. A grid of contrasting checks flips colour at a
//  speed you control, and can be warped: flat grid, bent/barrel, spiral, or a rotating "tunnel"
//  (downward spiral) for zoom-illusions. Cheap immediate-mode drawing with grain and chromatic
//  aberration on top. Colours come from black/white or the shared visualizer palettes. Audio-
//  reactive pass-through nudges the flip rate and warp. No feedback buffer -> stays smooth.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"

class CheckerBox : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
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

   void CellColor(int i, int j, int step, float& rOut, float& gOut, float& bOut) const;
   //draws the full checker for one colour channel pass (chroma aberration draws several offset passes)
   void DrawPattern(float viewX, float viewY, float viewW, float viewH, int step, float amp,
                    float offX, float offY, int channel, float alpha);

   //audio analysis
   float mAmplitude{ 0 };

   //controls
   float mSpeed{ 4.0f };
   int mGridN{ 10 };
   float mDistort{ 0.0f };
   int mPattern{ kPattern_Grid };
   float mChroma{ 0.25f };
   float mGrain{ 0.04f };
   float mSensitivity{ 1.0f };
   int mColorMode{ 0 }; //0 black/white, 1 palette
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 0 };

   FloatSlider* mSpeedSlider{ nullptr };
   IntSlider* mGridSlider{ nullptr };
   FloatSlider* mDistortSlider{ nullptr };
   DropdownList* mPatternSelector{ nullptr };
   FloatSlider* mChromaSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   FloatSlider* mSensitivitySlider{ nullptr };
   DropdownList* mColorModeSelector{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   float mWidth{ 360 };
   float mHeight{ 300 };
};
