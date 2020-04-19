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
, mFeedbackTarget(nullptr)
, mFeedbackTargetCable(nullptr)
, mFeedbackVizBuffer(VIZ_BUFFER_SECONDS*gSampleRate)
{
   AddChild(&mDelay);
   mDelay.SetPosition(3,15);
   mDelay.SetEnabled(true);
   mDelay.SetName("delay");
}

void FeedbackModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mFeedbackTargetCable = new PatchCableSource(this, kConnectionType_Audio);
   mFeedbackTargetCable->SetManualPosition(110, 8);
   mFeedbackTargetCable->SetOverrideCableDir(ofVec2f(0,1));
   mFeedbackTargetCable->SetOverrideVizBuffer(&mFeedbackVizBuffer);
   AddPatchCableSource(mFeedbackTargetCable);
   
   mDelay.CreateUIControls();
   mDelay.SetFeedbackModuleMode(true);
}

FeedbackModule::~FeedbackModule()
{
}

void FeedbackModule::Process(double time)
{
   PROFILER(FeedbackModule);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      if (GetTarget())
         Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
   
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),bufferSize,ch);
   }
   
   if (mFeedbackTarget)
   {
      mFeedbackTarget->GetBuffer()->SetNumActiveChannels(GetBuffer()->NumActiveChannels());
      mFeedbackVizBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());
      mDelay.ProcessAudio(gTime, GetBuffer());
      
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (mDelay.Enabled())
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
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["feedbacktarget"] = mFeedbackTarget ? dynamic_cast<IDrawableModule*>(mFeedbackTarget)->Name() : "";
}

void FeedbackModule::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mFeedbackTargetCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("feedbacktarget"),false));
}
