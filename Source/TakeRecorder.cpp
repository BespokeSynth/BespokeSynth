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
: mStartSeconds(0)
, mStartSecondsSlider(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

void TakeRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mStartSecondsSlider = new FloatSlider(this,"start",5,2,110,15,&mStartSeconds,0,4);
}

TakeRecorder::~TakeRecorder()
{
   delete[] mInputBuffer;
}

float* TakeRecorder::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void TakeRecorder::Process(double time)
{
   Profiler profiler("TakeRecorder");
   
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
   
   Clear(mInputBuffer, mInputBufferSize);
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
