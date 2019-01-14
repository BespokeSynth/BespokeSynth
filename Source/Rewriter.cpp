//
//  Rewriter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#include "Rewriter.h"
#include "Looper.h"
#include "ModularSynth.h"
#include "Transport.h"
#include "MidiController.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "AudioRouter.h"
#include "PatchCableSource.h"

Rewriter::Rewriter()
: IAudioProcessor(gBufferSize)
, mRewriteButton(nullptr)
, mConnectedLooper(nullptr)
, mRecordBuffer(MAX_BUFFER_SIZE)
, mStartRecordTime(-1)
, mStartRecordTimeButton(nullptr)
{
}

void Rewriter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRewriteButton = new ClickButton(this,"go",5,2);
   mStartRecordTimeButton = new ClickButton(this,"start",30,2);
   
   mLooperCable = new PatchCableSource(this,kConnectionType_Special);
   mLooperCable->SetManualPosition(70, 10);
   mLooperCable->AddTypeFilter("looper");
   AddPatchCableSource(mLooperCable);
}

Rewriter::~Rewriter()
{
}

void Rewriter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mLooperCable)
   {
      if (mConnectedLooper)
         mConnectedLooper->SetRewriter(nullptr);
      mConnectedLooper = dynamic_cast<Looper*>(mLooperCable->GetTarget());
      if (mConnectedLooper)
         mConnectedLooper->SetRewriter(this);
   }
}

void Rewriter::Process(double time)
{
   PROFILER(Rewriter);

   if (GetTarget() == nullptr)
      return;
   
   SyncBuffers();
   mRecordBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = GetBuffer()->BufferSize();

   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      mRecordBuffer.WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);

      Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);

      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void Rewriter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mRewriteButton->Draw();
   mStartRecordTimeButton->Draw();
}

void Rewriter::ButtonClicked(ClickButton *button)
{
   if (button == mStartRecordTimeButton)
      mStartRecordTime = gTime;
   if (button == mRewriteButton)
      Go();
}

void Rewriter::CheckboxUpdated(Checkbox* checkbox)
{
}

void Rewriter::Go()
{
   if (mConnectedLooper)
   {
      if (mStartRecordTime != -1)
      {
         int numBars = 1;
         float recordedTime = gTime - mStartRecordTime;
         int beats = numBars * TheTransport->GetTimeSigTop();
         float minutes = recordedTime / 1000.0f / 60.0f;
         float bpm = beats/minutes;
         if (bpm > 45 && bpm < 250)
         {
            TheTransport->SetTempo(bpm);
            TheTransport->SetDownbeat();
            mConnectedLooper->SetNumBars(1);
         }
         mStartRecordTime = -1;
      }

      mConnectedLooper->SetNumBars(mConnectedLooper->GetRecorderNumBars());
      mConnectedLooper->Commit(&mRecordBuffer);
      AudioRouter* connectedRouter = dynamic_cast<AudioRouter*>(mConnectedLooper->GetTarget());
      if (connectedRouter)
         connectedRouter->GetPatchCableSource()->SetTarget(this);
      TheSynth->ArrangeAudioSourceDependencies();
   }
}

void Rewriter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   mModuleSaveData.LoadString("looper",moduleInfo,"",FillDropdown<Looper*>);

   SetUpFromSaveData();
}

void Rewriter::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["looper"] = mConnectedLooper ? mConnectedLooper->Name() : "";
}

void Rewriter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   
   mLooperCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("looper"),false));
}

