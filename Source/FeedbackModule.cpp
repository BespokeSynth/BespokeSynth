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
: mFeedbackTarget(NULL)
, mFeedbackTargetCable(NULL)
, mFeedbackVizBuffer(VIZ_BUFFER_SECONDS*gSampleRate)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   
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
   delete[] mInputBuffer;
}

float* FeedbackModule::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void FeedbackModule::Process(double time)
{
   Profiler profiler("FeedbackModule");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = gBufferSize;
   if (GetTarget())
   {
      float* out = GetTarget()->GetBuffer(bufferSize);
      assert(bufferSize == gBufferSize);
      
      Add(out, mInputBuffer, bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);
   
   if (mFeedbackTarget)
   {
      float* out = mFeedbackTarget->GetBuffer(bufferSize);
      assert(bufferSize == gBufferSize);
      
      mDelay.ProcessAudio(gTime, mInputBuffer, mInputBufferSize);
      
      if (mDelay.Enabled())
      {
         Add(out, mInputBuffer, bufferSize);
         mFeedbackVizBuffer.WriteChunk(mInputBuffer, mInputBufferSize);
      }
      else
      {
         mFeedbackVizBuffer.WriteChunk(gZeroBuffer, gBufferSize);
      }
   }
   
   Clear(mInputBuffer, mInputBufferSize);
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
