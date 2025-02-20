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
   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override {}

   //ITextEntryListener
   void TextEntryComplete(TextEntry* entry) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& w, double& h) override;

   double mRescale{ 1 };
   FloatSlider* mRescaleSlider{ nullptr };
   double mA{ 0 };
   FloatSlider* mASlider{ nullptr };
   double mB{ 0 };
   FloatSlider* mBSlider{ nullptr };
   double mC{ 0 };
   FloatSlider* mCSlider{ nullptr };
   double mD{ 0 };
   FloatSlider* mDSlider{ nullptr };
   double mE{ 0 };
   FloatSlider* mESlider{ nullptr };

   std::string mEntryString{ "x" };
   TextEntry* mTextEntry{ nullptr };
   exprtk::symbol_table<double> mSymbolTable;
   exprtk::expression<double> mExpression;
   exprtk::symbol_table<double> mSymbolTableDraw;
   exprtk::expression<double> mExpressionDraw;

   double mExpressionInput{ 0 };
   double mHistPre1{ 0 };
   double mHistPre2{ 0 };
   double mHistPost1{ 0 };
   double mHistPost2{ 0 };
   double mExpressionInputDraw{ 0 };
   double mT{ 0 };
   bool mExpressionValid{ false };
   double mSmoothMax{ 0 };
   double mSmoothMin{ 0 };

   struct BiquadState
   {
      double mHistPre1{ 0 };
      double mHistPre2{ 0 };
      double mHistPost1{ 0 };
      double mHistPost2{ 0 };
   };

   BiquadState mBiquadState[ChannelBuffer::kMaxNumChannels];
};
