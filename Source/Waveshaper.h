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
#include "exprtk.hpp"

class Waveshaper : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public ITextEntryListener
{
public:
   Waveshaper();
   virtual ~Waveshaper();
   static IDrawableModule* Create() { return new Waveshaper(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   //ITextEntryListener
   void TextEntryComplete(TextEntry* entry) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;

   float mRescale{ 1 };
   FloatSlider* mRescaleSlider{ nullptr };
   float mA{ 0 };
   FloatSlider* mASlider{ nullptr };
   float mB{ 0 };
   FloatSlider* mBSlider{ nullptr };
   float mC{ 0 };
   FloatSlider* mCSlider{ nullptr };
   float mD{ 0 };
   FloatSlider* mDSlider{ nullptr };
   float mE{ 0 };
   FloatSlider* mESlider{ nullptr };

   std::string mEntryString{ "x" };
   TextEntry* mTextEntry{ nullptr };
   exprtk::symbol_table<float> mSymbolTable;
   exprtk::expression<float> mExpression;
   exprtk::symbol_table<float> mSymbolTableDraw;
   exprtk::expression<float> mExpressionDraw;

   float mExpressionInput{ 0 };
   float mHistPre1{ 0 };
   float mHistPre2{ 0 };
   float mHistPost1{ 0 };
   float mHistPost2{ 0 };
   float mExpressionInputDraw{ 0 };
   float mT{ 0 };
   bool mExpressionValid{ false };
   float mSmoothMax{ 0 };
   float mSmoothMin{ 0 };

   struct BiquadState
   {
      float mHistPre1{ 0 };
      float mHistPre2{ 0 };
      float mHistPost1{ 0 };
      float mHistPost2{ 0 };
   };

   BiquadState mBiquadState[ChannelBuffer::kMaxNumChannels];
};
