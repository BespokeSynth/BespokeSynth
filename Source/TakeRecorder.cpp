/*
  ==============================================================================

    TakeRecorder.cpp
    Created: 9 Aug 2017 11:31:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "TakeRecorder.h"
#include "ModularSynth.h"
#include "Profiler.h"

TakeRecorder::TakeRecorder()
: IAudioProcessor(gBufferSize)
, mStartSeconds(0)
, mStartSecondsSlider(nullptr)
{
}

void TakeRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mStartSecondsSlider = new FloatSlider(this,"start",5,2,110,15,&mStartSeconds,0,4);
}

TakeRecorder::~TakeRecorder()
{
}

void TakeRecorder::Process(double time)
{
   PROFILER(TakeRecorder);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      Add(GetTarget()->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize, 0);
   
   GetBuffer()->Reset();
}

void TakeRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mStartSecondsSlider->Draw();
}

void TakeRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void TakeRecorder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
