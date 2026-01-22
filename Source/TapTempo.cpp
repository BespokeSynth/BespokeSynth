/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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

    TapTempo.cpp
    Created: 26 Sep 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "TapTempo.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Checkbox.h"
#include "Transport.h"
#include "OpenFrameworksPort.h"

#include "juce_gui_basics/juce_gui_basics.h"

TapTempo::TapTempo()
{
}

TapTempo::~TapTempo()
{
}

void TapTempo::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   BUTTON(mTapButton, "tap");
   UIBLOCK_SHIFTY(90);
   BUTTON(mSendTempoButton, "send tempo");
   CHECKBOX(mRoundTempoCheckbox, "round tempo", &mRoundTempo);
   ENDUIBLOCK(mWidth, mHeight);

   mTapButton->SetDimensions(130, 100);

   mWidth = 140;
   mHeight += 34;
}

void TapTempo::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTapButton->Draw();
   mSendTempoButton->Draw();
   mRoundTempoCheckbox->Draw();

   if (mTapTempoDetector.HasEnoughSamples())
   {
      float y = mRoundTempoCheckbox->GetRect(K(local)).getMaxY() + 14;
      if (mRoundTempo)
      {
         float roundedTempo = round(mTapTempoDetector.GetCalculatedTempo());
         float diff = mTapTempoDetector.GetCalculatedTempo() - roundedTempo;
         DrawTextNormal("tempo: " + ofToString(roundedTempo, 2) + " (" + ofToString(diff, 2) + ")", 5, y);
      }
      else
      {
         DrawTextNormal("tempo: " + ofToString(mTapTempoDetector.GetCalculatedTempo(), 2), 5, y);
      }
      DrawTextNormal("std dev: " + ofToString(mTapTempoDetector.GetCalculationStandardDeviation(), 2), 5, y + 14);
   }
}

void TapTempo::DrawModuleUnclipped()
{
   if (mDrawDebug)
      DrawTextNormal(mDebugDisplayText, 0, mHeight + 20);
}

void TapTempo::OnKeyPressed(int key, bool isRepeat)
{
   if (key == ' ')
      ButtonClicked(mTapButton, gTime);
   if (key == OF_KEY_RETURN)
      ButtonClicked(mSendTempoButton, gTime);
}

void TapTempo::OnPulse(double time, float velocity, int flags)
{
   ButtonClicked(mTapButton, time);
}

void TapTempo::ButtonClicked(ClickButton* button, double time)
{
   if (button == mTapButton)
   {
      mTapTempoDetector.Tap(time);
   }

   if (button == mSendTempoButton)
   {
      if (mRoundTempo)
         TheTransport->SetTempo(round(mTapTempoDetector.GetCalculatedTempo()));
      else
         TheTransport->SetTempo(mTapTempoDetector.GetCalculatedTempo());
      TheTransport->Reset(K(timeSensitive));
   }
}

void TapTempo::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void TapTempo::SetUpFromSaveData()
{
}

void TapTempoDetector::Tap(double time)
{
   if (mNumAccumulatedSamples > 0)
   {
      double lastTime = mTimeSamples[mLastTimeSamplesWriteIdx];
      constexpr float kRestartThresholdMs = 2000; // 30 bpm
      if (time - lastTime > kRestartThresholdMs)
      {
         for (size_t i = 0; i < mTimeSamples.size(); ++i)
            mTimeSamples[i] = 0.0;
         mNumAccumulatedSamples = 0;
      }
   }
   int writeIdx = (mLastTimeSamplesWriteIdx + 1) % mTimeSamples.size();
   mTimeSamples[writeIdx] = time;
   mLastTimeSamplesWriteIdx = writeIdx;
   mNumAccumulatedSamples = std::min(mNumAccumulatedSamples + 1, (int)mTimeSamples.size());

   CalculateTempo();
}

void TapTempoDetector::CalculateTempo()
{
   if (mNumAccumulatedSamples <= 1)
      return;

   std::vector<double> deltas;
   for (int i = 1; i < mNumAccumulatedSamples; ++i)
   {
      int index = (i + mLastTimeSamplesWriteIdx - mNumAccumulatedSamples + 1 + (int)mTimeSamples.size()) % (int)mTimeSamples.size();
      int prevIndex = (index - 1 + (int)mTimeSamples.size()) % (int)mTimeSamples.size();
      deltas.push_back(mTimeSamples[index] - mTimeSamples[prevIndex]);
      //ofLog() << ofToString(index) << " " << ofToString(prevIndex) << " " << ofToString(mTimeSamples[index]) << " " << ofToString(mTimeSamples[prevIndex]) << " " << ofToString(deltas[deltas.size() - 1]);
   }
   std::sort(deltas.begin(), deltas.end());

   int numDeltas = (int)deltas.size();
   int outliers = std::max(0, (numDeltas - 4) / 2);
   int numSamples = ((int)deltas.size() - outliers * 2);

   /*if (mDrawDebug)
   {
      mDebugDisplayText = "";
      for (int i = 0; i < (int)deltas.size(); ++i)
      {
         float sampleTempo = 60.0 / (deltas[i] / 1000.0);
         mDebugDisplayText += ofToString(sampleTempo, 2);
         if (i < outliers || i >= deltas.size() - outliers)
            mDebugDisplayText += "(X)";
         mDebugDisplayText += "\n";
      }
   }*/

   double averageTimeDeltaMs = 0.0;
   for (int i = outliers; i < (int)deltas.size() - outliers; ++i)
      averageTimeDeltaMs += deltas[i] / numSamples;

   double tempo = 60.0 / (averageTimeDeltaMs / 1000.0);

   mLastCalculatedTempo = (float)tempo;

   double variance = 0.0;
   for (int i = outliers; i < (int)deltas.size() - outliers; ++i)
   {
      double deviation = averageTimeDeltaMs - deltas[i];
      variance += (deviation * deviation) / numSamples;
   }

   mStandardDeviation = sqrtf((float)variance);
}
