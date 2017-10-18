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
, mFeedbackTarget(NULL)
, mFeedbackTargetCable(NULL)
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
   Profiler profiler("FeedbackModule");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
      Add(GetTarget()->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize);
   
   if (mFeedbackTarget)
   {
      float* out = mFeedbackTarget->GetBuffer()->GetChannel(0);
      assert(bufferSize == gBufferSize);
      
      mDelay.ProcessAudio(gTime, GetBuffer()->GetChannel(0), bufferSize);
      
      if (mDelay.Enabled())
      {
         Add(out, GetBuffer()->GetChannel(0), bufferSize);
         mFeedbackVizBuffer.WriteChunk(GetBuffer()->GetChannel(0), bufferSize);
      }
      else
      {
         mFeedbackVizBuffer.WriteChunk(gZeroBuffer, gBufferSize);
      }
   }
   
   GetBuffer()->Clear();
}

void FeedbackModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mDelay.Draw();
}

void FeedbackModule::PostRepatch(PatchCableSource* cable)
{
   if (cable == mFeedbackTargetCable)
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
