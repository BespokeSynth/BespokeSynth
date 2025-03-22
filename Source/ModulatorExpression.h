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

    ModulatorExpression.h
    Created: 8 May 2021 5:36:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IModulator.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "exprtk.hpp"

class ModulatorExpression : public IDrawableModule, public IFloatSliderListener, public ITextEntryListener, public IModulator
{
public:
   ModulatorExpression();
   virtual ~ModulatorExpression();
   static IDrawableModule* Create() { return new ModulatorExpression(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

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

   float mExpressionInput{ 0 };
   FloatSlider* mExpressionInputSlider{ nullptr };
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

   float mExpressionInputDraw{ 0 };
   float mT{ 0 };
   bool mExpressionValid{ false };
   float mLastDrawMinOutput{ 0 };
   float mLastDrawMaxOutput{ 1 };
};
