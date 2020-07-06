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
   
   string GetTitleLabel() override { return "waveshaper"; }
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
   
   char mEntryString[MAX_TEXTENTRY_LENGTH];
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
