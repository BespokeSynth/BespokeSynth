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
//  Lissajous.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#ifndef __Bespoke__Lissajous__
#define __Bespoke__Lissajous__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"

#define NUM_LISSAJOUS_POINTS 3000

class Lissajous : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   Lissajous();
   virtual ~Lissajous();
   static IDrawableModule* Create() { return new Lissajous(); }


   void CreateUIControls() override;

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;
   float mScale;
   FloatSlider* mScaleSlider;

   ofVec2f mLissajousPoints[NUM_LISSAJOUS_POINTS];
   int mOffset;
   bool mAutocorrelationMode;
   bool mOnlyHasOneChannel;
};


#endif /* defined(__Bespoke__Lissajous__) */
