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
//  FeedbackModule.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/1/16.
//
//

#include "FeedbackModule.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "ModularSynth.h"

FeedbackModule::FeedbackModule()
: IAudioProcessor(gBufferSize)
, mFeedbackVizBuffer(VIZ_BUFFER_SECONDS * gSampleRate)
{
   AddChild(&mDelay);
   mDelay.SetPosition(4, 32);
   mDelay.SetEnabled(true);
   mDelay.SetName("delay");

   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
      mGainScale[i] = 1;
}

void FeedbackModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mFeedbackTargetCable = new PatchCableSource(this, kConnectionType_Audio);
   mFeedbackTargetCable->SetManualPosition(108, 8);
   mFeedbackTargetCable->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   mFeedbackTargetCable->SetOverrideVizBuffer(&mFeedbackVizBuffer);
   AddPatchCableSource(mFeedbackTargetCable);

   mDelay.CreateUIControls();
   mDelay.SetFeedbackModuleMode();

   ofRectangle delayModuleRect = mDelay.GetRect(true);
   mSignalLimitSlider = new FloatSlider(this, "limit", delayModuleRect.x, delayModuleRect.getMaxY() + 3, delayModuleRect.width, 15, &mSignalLimit, 0.01f, 1);
   mSignalLimitSlider->SetMode(FloatSlider::kSquare);
}

FeedbackModule::~FeedbackModule()
{
}

void FeedbackModule::Process(double time)
{
   PROFILER(FeedbackModule);


   ComputeSliders(0);
   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   IAudioReceiver* target = GetTarget();

   if (target)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
      }
   }

   if (mFeedbackTarget && mEnabled)
   {
      mFeedbackTarget->GetBuffer()->SetNumActiveChannels(GetBuffer()->NumActiveChannels());
      mFeedbackVizBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());
      mDelay.ProcessAudio(time, GetBuffer());

      const double kReleaseMs = 50;
      const double kReleaseCoeff = exp(-1000.0 / (kReleaseMs * gSampleRate));
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         float* channel = GetBuffer()->GetChannel(ch);
         for (int i = 0; i < bufferSize; ++i)
         {
            //ofLog() << "input: " << channel[i];
            if (abs(channel[i]) * mGainScale[ch] > mSignalLimit)
            {
               mGainScale[ch] = mSignalLimit / abs(channel[i]); //limit immediately
               //ofLog() << "limiting, new scale " << mGainScale[ch];
            }
            else
            {
               mGainScale[ch] = MIN(1 - kReleaseCoeff * (mGainScale[ch] - 1), //blend towards 1
                                    mSignalLimit / abs(channel[i])); //but don't make the scale less than it would have to be to limit the output
               //ofLog() << "releasing, new scale " << mGainScale[ch];
            }

            channel[i] *= mGainScale[ch];
            //ofLog() << "output: " << channel[i];
            //assert(abs(channel[i]) <= mSignalLimit);
         }

         if (mDelay.IsEnabled())
         {
            Add(mFeedbackTarget->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
            mFeedbackVizBuffer.WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
         }
         else
         {
            mFeedbackVizBuffer.WriteChunk(gZeroBuffer, gBufferSize, ch);
         }
      }
   }

   GetBuffer()->Reset();
}

void FeedbackModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDelay.Draw();
   mSignalLimitSlider->Draw();

   DrawTextRightJustify("feedback out:", 100, 12);
}

void FeedbackModule::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mFeedbackTargetCable)
   {
      mFeedbackTarget = mFeedbackTargetCable->GetAudioReceiver();
   }
}

void FeedbackModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("feedbacktarget", moduleInfo);

   SetUpFromSaveData();
}

void FeedbackModule::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["feedbacktarget"] = mFeedbackTarget ? dynamic_cast<IDrawableModule*>(mFeedbackTarget)->Name() : "";
}

void FeedbackModule::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mFeedbackTargetCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("feedbacktarget"), false));
}
