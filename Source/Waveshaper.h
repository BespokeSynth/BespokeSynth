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

    Waveshaper.h
    Created: 1 Dec 2019 10:01:56pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "exprtk/exprtk.hpp"

class Waveshaper : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public ITextEntryListener
{
public:
   Waveshaper();
   virtual ~Waveshaper();
   static IDrawableModule* Create() { return new Waveshaper(); }
   
   std::string GetTitleLabel() override { return "waveshaper"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   //ITextEntryListener
   void TextEntryComplete(TextEntry* entry) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   bool Enabled() const override { return mEnabled; }
   
   float mRescale;
   FloatSlider* mRescaleSlider;
   float mA;
   FloatSlider* mASlider;
   float mB;
   FloatSlider* mBSlider;
   float mC;
   FloatSlider* mCSlider;
   float mD;
   FloatSlider* mDSlider;
   float mE;
   FloatSlider* mESlider;
   
   std::string mEntryString;
   TextEntry* mTextEntry;
   exprtk::symbol_table<float> mSymbolTable;
   exprtk::expression<float> mExpression;
   exprtk::symbol_table<float> mSymbolTableDraw;
   exprtk::expression<float> mExpressionDraw;
   
   float mExpressionInput;
   float mHistPre1;
   float mHistPre2;
   float mHistPost1;
   float mHistPost2;
   float mExpressionInputDraw;
   float mT;
   bool mExpressionValid;
   float mSmoothMax;
   float mSmoothMin;
   
   struct BiquadState
   {
      BiquadState()
      : mHistPre1(0)
      , mHistPre2(0)
      , mHistPost1(0)
      , mHistPost2(0)
      {}
      float mHistPre1;
      float mHistPre2;
      float mHistPost1;
      float mHistPost2;
   };
   
   BiquadState mBiquadState[ChannelBuffer::kMaxNumChannels];
};
