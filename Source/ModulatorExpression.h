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
#include "exprtk/exprtk.hpp"

class ModulatorExpression : public IDrawableModule, public IFloatSliderListener, public ITextEntryListener, public IModulator
{
public:
   ModulatorExpression();
   virtual ~ModulatorExpression();
   static IDrawableModule* Create() { return new ModulatorExpression(); }
   
   string GetTitleLabel() override { return "expression"; }
   void CreateUIControls() override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
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
   
   float mExpressionInput;
   FloatSlider* mExpressionInputSlider;
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
   
   char mEntryString[MAX_TEXTENTRY_LENGTH];
   TextEntry* mTextEntry;
   exprtk::symbol_table<float> mSymbolTable;
   exprtk::expression<float> mExpression;
   exprtk::symbol_table<float> mSymbolTableDraw;
   exprtk::expression<float> mExpressionDraw;
   
   float mExpressionInputDraw;
   float mT;
   bool mExpressionValid;
   float mLastDrawMinOutput;
   float mLastDrawMaxOutput;
};
