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

    ModulatorExpression.cpp
    Created: 8 May 2021 5:37:01pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorExpression.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Profiler.h"

namespace
{
   const int kGraphWidth = 100;
   const int kGraphHeight = 100;
   const int kGraphX = 115;
   const int kGraphY = 18;
}

ModulatorExpression::ModulatorExpression()
{
}

void ModulatorExpression::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTextEntry = new TextEntry(this, "y=", 2, 2, MAX_TEXTENTRY_LENGTH - 1, &mEntryString);
   mTextEntry->SetFlexibleWidth(true);
   mTextEntry->DrawLabel(true);
   mExpressionInputSlider = new FloatSlider(this, "input", mTextEntry, kAnchor_Below, 110, 15, &mExpressionInput, 0, 1);
   mASlider = new FloatSlider(this, "a", mExpressionInputSlider, kAnchor_Below, 110, 15, &mA, -10, 10, 4);
   mBSlider = new FloatSlider(this, "b", mASlider, kAnchor_Below, 110, 15, &mB, -10, 10, 4);
   mCSlider = new FloatSlider(this, "c", mBSlider, kAnchor_Below, 110, 15, &mC, -10, 10, 4);
   mDSlider = new FloatSlider(this, "d", mCSlider, kAnchor_Below, 110, 15, &mD, -10, 10, 4);
   mESlider = new FloatSlider(this, "e", mDSlider, kAnchor_Below, 110, 15, &mE, -10, 10, 4);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);

   mSymbolTable.add_variable("x", mExpressionInput);
   mSymbolTable.add_variable("t", mT);
   mSymbolTable.add_variable("a", mA);
   mSymbolTable.add_variable("b", mB);
   mSymbolTable.add_variable("c", mC);
   mSymbolTable.add_variable("d", mD);
   mSymbolTable.add_variable("e", mE);
   mSymbolTable.add_constants();
   mExpression.register_symbol_table(mSymbolTable);

   mSymbolTableDraw.add_variable("x", mExpressionInputDraw);
   mSymbolTableDraw.add_variable("t", mT);
   mSymbolTableDraw.add_variable("a", mA);
   mSymbolTableDraw.add_variable("b", mB);
   mSymbolTableDraw.add_variable("c", mC);
   mSymbolTableDraw.add_variable("d", mD);
   mSymbolTableDraw.add_variable("e", mE);
   mSymbolTableDraw.add_constants();
   mExpressionDraw.register_symbol_table(mSymbolTableDraw);

   TextEntryComplete(mTextEntry);
}

ModulatorExpression::~ModulatorExpression()
{
}

float ModulatorExpression::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   if (mExpressionValid)
   {
      mT = (gTime + samplesIn * gInvSampleRateMs) * .001;
      return mExpression.value();
   }

   if (GetSliderTarget())
      return GetSliderTarget()->GetMin();
   return 0;
}

void ModulatorExpression::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (GetSliderTarget() && fromUserClick)
   {
      //mValue1Slider->SetExtents(mSliderTarget->GetMin(), mSliderTarget->GetMax());
      //mValue1Slider->SetMode(mSliderTarget->GetMode());
   }
}

void ModulatorExpression::TextEntryComplete(TextEntry* entry)
{
   mExpressionValid = false;
   exprtk::parser<float> parser;
   mExpressionValid = parser.compile(mEntryString, mExpression);
   if (mExpressionValid)
      parser.compile(mEntryString, mExpressionDraw);
}

void ModulatorExpression::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();

   ofPushStyle();
   ofSetColor(80, 80, 80, 80 * gModuleDrawAlpha);
   ofFill();
   ofRect(kGraphX, kGraphY, kGraphWidth, kGraphHeight);
   ofPopStyle();

   if (!mExpressionValid)
   {
      ofSetColor(255, 0, 0, 60);
      ofFill();
      ofRect(mTextEntry->GetRect(true));
   }
   else
   {
      ofPushMatrix();
      ofClipWindow(kGraphX, kGraphY, kGraphWidth, kGraphHeight, true);
      ofPushStyle();
      ofSetColor(0, 255, 0, gModuleDrawAlpha);
      ofNoFill();
      ofBeginShape();
      float drawMinOutput = mLastDrawMinOutput;
      float drawMaxOutput = mLastDrawMaxOutput;
      for (int i = 0; i <= kGraphWidth; ++i)
      {
         mExpressionInputDraw = ofMap(i, 0, kGraphWidth, mExpressionInputSlider->GetMin(), mExpressionInputSlider->GetMax());
         float output = mExpressionDraw.value();
         ofVertex(i + kGraphX, ofMap(output, drawMinOutput, drawMaxOutput, kGraphHeight, 0) + kGraphY);

         if (i == 0)
         {
            mLastDrawMinOutput = output;
            mLastDrawMaxOutput = output;
         }
         else
         {
            if (output < mLastDrawMinOutput)
               mLastDrawMinOutput = output;
            if (output > mLastDrawMaxOutput)
               mLastDrawMaxOutput = output;
         }
      }
      ofEndShape();

      ofSetColor(245, 58, 135);
      mExpressionInputDraw = mExpressionInput;
      ofCircle(kGraphX + ofMap(mExpressionInputDraw, mExpressionInputSlider->GetMin(), mExpressionInputSlider->GetMax(), 0, kGraphWidth), ofMap(mExpressionDraw.value(), mLastDrawMinOutput, mLastDrawMaxOutput, kGraphHeight, 0) + kGraphY, 3);
      ofPopStyle();

      DrawTextNormal(ofToString(drawMinOutput, 2), kGraphX + kGraphWidth * .35f, kGraphY + kGraphHeight - 1);
      DrawTextNormal(ofToString(drawMaxOutput, 2), kGraphX + kGraphWidth * .35f, kGraphY + 12);
      DrawTextNormal(ofToString(mExpressionInputSlider->GetMin()), kGraphX + 1, kGraphY + kGraphHeight / 2 + 6);
      DrawTextRightJustify(ofToString(mExpressionInputSlider->GetMax()), kGraphX + kGraphWidth - 1, kGraphY + kGraphHeight / 2 + 6);

      ofPopMatrix();
   }

   ofPopStyle();

   mTextEntry->Draw();
   mExpressionInputSlider->Draw();
   mASlider->Draw();
   mBSlider->Draw();
   mCSlider->Draw();
   mDSlider->Draw();
   mESlider->Draw();
}

void ModulatorExpression::GetModuleDimensions(float& w, float& h)
{
   w = MAX(kGraphX + kGraphWidth + 2, 4 + mTextEntry->GetRect().width);
   h = kGraphY + kGraphHeight;
}

void ModulatorExpression::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorExpression::SetUpFromSaveData()
{
}
