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
#include "PatchCableSource.h"
#include "AudioSend.h"
#include "UIControlMacros.h"

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
   UIBLOCK0();
   BUTTON(mRewriteButton, " go "); UIBLOCK_SHIFTRIGHT();
   BUTTON(mStartRecordTimeButton,"new loop");
   ENDUIBLOCK(mWidth, mHeight);
   
   mLooperCable = new PatchCableSource(this,kConnectionType_Special);
   mLooperCable->SetManualPosition(99, 10);
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

   if (mStartRecordTime != -1)
   {
      ofSetColor(255, 100, 0, 100 + 50 * (cosf(TheTransport->GetMeasurePos(gTime) * 4 * FTWO_PI)));
      ofRect(mStartRecordTimeButton->GetRect(true));
   }
}

void Rewriter::ButtonClicked(ClickButton *button)
{
   if (button == mStartRecordTimeButton)
   {
      if (mStartRecordTime == -1)
         mStartRecordTime = gTime + gBufferSizeMs;
      else
         mStartRecordTime = -1;
   }
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
         float recordedMs = gTime + gBufferSizeMs - mStartRecordTime;
         float numBarsCurrentTempo = recordedMs / TheTransport->MsPerBar();
         int numBars = int(numBarsCurrentTempo + .5f);
         numBars = MAX(1, int(Pow2(floor(log2(numBars)))));   //find closest power of 2
         
         int beats = numBars * TheTransport->GetTimeSigTop();
         float minutes = recordedMs / 1000.0f / 60.0f;
         float bpm = beats / minutes;
         TheTransport->SetTempo(bpm);
         TheTransport->SetDownbeat();
         mConnectedLooper->SetNumBars(numBars);
         mStartRecordTime = -1;
      }

      mConnectedLooper->SetNumBars(mConnectedLooper->GetRecorderNumBars());
      mConnectedLooper->Commit(&mRecordBuffer);
      AudioSend* connectedSend = dynamic_cast<AudioSend*>(mConnectedLooper->GetTarget());
      if (connectedSend)
         connectedSend->SetSend(1, true);
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

