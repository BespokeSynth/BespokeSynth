/*
  ==============================================================================

    DCOffset.cpp
    Created: 1 Dec 2019 3:24:31pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "DCOffset.h"
#include "ModularSynth.h"
#include "Profiler.h"

DCOffset::DCOffset()
: IAudioProcessor(gBufferSize)
, mOffset(0)
, mOffsetSlider(nullptr)
{
}

void DCOffset::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mOffsetSlider = new FloatSlider(this,"offset",5,2,110,15,&mOffset,-1,1);
}

DCOffset::~DCOffset()
{
}

void DCOffset::Process(double time)
{
   PROFILER(DCOffset);

   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   if (GetTarget())
   {
      int bufferSize = GetBuffer()->BufferSize();
      
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         float* buffer = GetBuffer()->GetChannel(ch);
         for (int i=0; i<bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer[i] += mOffset;
         }
         Add(out->GetChannel(ch), buffer, bufferSize);
         GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
      }
   }
   
   GetBuffer()->Reset();
}

void DCOffset::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mOffsetSlider->Draw();
}

void DCOffset::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void DCOffset::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
