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

    Waveshaper.cpp
    Created: 1 Dec 2019 10:01:56pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Waveshaper.h"
#include "ModularSynth.h"
#include "Profiler.h"

namespace
{
   const int kGraphWidth = 100;
   const int kGraphHeight = 100;
   const int kGraphX = 115;
   const int kGraphY = 18;
}

Waveshaper::Waveshaper()
: IAudioProcessor(gBufferSize)
{
}

void Waveshaper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTextEntry = new TextEntry(this, "y=", 2, 2, MAX_TEXTENTRY_LENGTH - 1, &mEntryString);
   mTextEntry->SetFlexibleWidth(true);
   mTextEntry->DrawLabel(true);
   mRescaleSlider = new FloatSlider(this, "rescale", mTextEntry, kAnchor_Below, 110, 15, &mRescale, .1f, 10);
   mASlider = new FloatSlider(this, "a", mRescaleSlider, kAnchor_Below, 110, 15, &mA, -10, 10, 4);
   mBSlider = new FloatSlider(this, "b", mASlider, kAnchor_Below, 110, 15, &mB, -10, 10, 4);
   mCSlider = new FloatSlider(this, "c", mBSlider, kAnchor_Below, 110, 15, &mC, -10, 10, 4);
   mDSlider = new FloatSlider(this, "d", mCSlider, kAnchor_Below, 110, 15, &mD, -10, 10, 4);
   mESlider = new FloatSlider(this, "e", mDSlider, kAnchor_Below, 110, 15, &mE, -10, 10, 4);

   mSymbolTable.add_variable("x", mExpressionInput);
   mSymbolTable.add_variable("x1", mHistPre1);
   mSymbolTable.add_variable("x2", mHistPre2);
   mSymbolTable.add_variable("y1", mHistPost1);
   mSymbolTable.add_variable("y2", mHistPost2);
   mSymbolTable.add_variable("t", mT);
   mSymbolTable.add_variable("a", mA);
   mSymbolTable.add_variable("b", mB);
   mSymbolTable.add_variable("c", mC);
   mSymbolTable.add_variable("d", mD);
   mSymbolTable.add_variable("e", mE);
   mSymbolTable.add_constants();
   mExpression.register_symbol_table(mSymbolTable);

   mSymbolTableDraw.add_variable("x", mExpressionInputDraw);
   mSymbolTableDraw.add_variable("x1", mExpressionInputDraw);
   mSymbolTableDraw.add_variable("x2", mExpressionInputDraw);
   mSymbolTableDraw.add_variable("y1", mExpressionInputDraw);
   mSymbolTableDraw.add_variable("y2", mExpressionInputDraw);
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

Waveshaper::~Waveshaper()
{
}

void Waveshaper::Process(double time)
{
   PROFILER(Waveshaper);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   float max = 0;
   float min = 0;

   int bufferSize = GetBuffer()->BufferSize();

   ChannelBuffer* out = target->GetBuffer();
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      float* buffer = GetBuffer()->GetChannel(ch);
      if (mExpressionValid)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            mExpressionInput = buffer[i] * mRescale;

            mHistPre1 = mBiquadState[ch].mHistPre1;
            mHistPre2 = mBiquadState[ch].mHistPre2;
            mHistPost1 = mBiquadState[ch].mHistPost1;
            mHistPost2 = mBiquadState[ch].mHistPost2;

            if (mExpressionInput > max)
               max = mExpressionInput;
            if (mExpressionInput < min)
               min = mExpressionInput;

            mT = (gTime + i * gInvSampleRateMs) * .001;
            buffer[i] = mExpression.value() / mRescale;

            mBiquadState[ch].mHistPre2 = mBiquadState[ch].mHistPre1;
            mBiquadState[ch].mHistPre1 = mExpressionInput;
            mBiquadState[ch].mHistPost2 = mBiquadState[ch].mHistPost1;
            mBiquadState[ch].mHistPost1 = ofClamp(buffer[i], -1, 1); //keep feedback from spiraling out of control
         }
      }
      Add(out->GetChannel(ch), buffer, bufferSize);
      GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
   }

   mSmoothMax = max > mSmoothMax ? max : ofLerp(mSmoothMax, max, .01f);
   mSmoothMin = min < mSmoothMin ? min : ofLerp(mSmoothMin, min, .01f);

   GetBuffer()->Reset();
}

void Waveshaper::TextEntryComplete(TextEntry* entry)
{
   exprtk::parser<float> parser;
   mExpressionValid = parser.compile(mEntryString, mExpression);
   if (mExpressionValid)
      parser.compile(mEntryString, mExpressionDraw);
}

void Waveshaper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();

   ofSetColor(80, 80, 80, 80 * gModuleDrawAlpha);
   ofFill();
   ofRect(kGraphX, kGraphY, kGraphWidth, kGraphHeight);

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
      ofSetColor(0, 255, 0, gModuleDrawAlpha);
      ofNoFill();
      ofBeginShape();
      for (int i = 0; i < 100; ++i)
      {
         mExpressionInputDraw = ofMap(i, 0, kGraphWidth, -1, 1);
         ofVertex(i + kGraphX, ofMap(mExpressionDraw.value(), -1, 1, kGraphHeight, 0) + kGraphY);
      }
      ofEndShape();

      ofSetColor(245, 58, 135);
      mExpressionInputDraw = mSmoothMin;
      ofCircle(kGraphX + ofMap(mSmoothMin, -1, 1, 0, kGraphWidth), ofMap(mExpressionDraw.value(), -1, 1, kGraphHeight, 0) + kGraphY, 3);
      mExpressionInputDraw = mSmoothMax;
      ofCircle(kGraphX + ofMap(mSmoothMax, -1, 1, 0, kGraphWidth), ofMap(mExpressionDraw.value(), -1, 1, kGraphHeight, 0) + kGraphY, 3);

      ofPopMatrix();
   }

   ofPopStyle();

   mTextEntry->Draw();
   mRescaleSlider->Draw();
   mASlider->Draw();
   mBSlider->Draw();
   mCSlider->Draw();
   mDSlider->Draw();
   mESlider->Draw();
}

void Waveshaper::GetModuleDimensions(float& w, float& h)
{
   w = MAX(kGraphX + kGraphWidth + 2, 4 + mTextEntry->GetRect().width);
   h = kGraphY + kGraphHeight;
}

void Waveshaper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Waveshaper::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
