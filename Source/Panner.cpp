/*
  ==============================================================================

    Panner.cpp
    Created: 10 Oct 2017 9:49:17pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Panner.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Panner::Panner()
: IAudioProcessor(gBufferSize)
, mPan(0)
, mPanSlider(nullptr)
, mWiden(0)
, mWidenSlider(nullptr)
, mWidenerBuffer(2048)
{
}

void Panner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this,"pan",5,20,110,15,&mPan,-1,1);
   mWidenSlider = new FloatSlider(this,"widen",55,2,60,15,&mWiden,-150,150,0);
}

Panner::~Panner()
{
}

void Panner::Process(double time)
{
   PROFILER(Panner);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
 
   SyncBuffers(2);
   mWidenerBuffer.SetNumChannels(2);
   
   float* secondChannel;
   if (GetBuffer()->NumActiveChannels() == 1)   //panning mono input
   {
      BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
      secondChannel = gWorkBuffer;
   }
   else
   {
      secondChannel = GetBuffer()->GetChannel(1);
   }
   
   ChannelBuffer* out = GetTarget()->GetBuffer();
   
   if (abs(mWiden) > 0)
   {
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
         mWidenerBuffer.WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      if (mWiden < 0)
         mWidenerBuffer.ReadChunk(secondChannel, GetBuffer()->BufferSize(), abs(mWiden), 1);
      else
         mWidenerBuffer.ReadChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), abs(mWiden), 0);
   }
   
   for (int i=0; i<GetBuffer()->BufferSize(); ++i)
   {
      ComputeSliders(i);
      mPanRamp.Start(mPan,2);
      float pan = mPanRamp.Value(time);
      
      float left = GetBuffer()->GetChannel(0)[i];
      float right = secondChannel[i];
      GetBuffer()->GetChannel(0)[i] = left * ofMap(pan, 0, 1, 1, 0, true) + right * ofMap(pan, -1, 0, 1, 0, true);
      secondChannel[i] = right * ofMap(pan, -1, 0, 0, 1, true) + left * ofMap(pan, 0, 1, 0, 1, true);
      
      out->GetChannel(0)[i] += GetBuffer()->GetChannel(0)[i];
      out->GetChannel(1)[i] += secondChannel[i];
      
      time += gInvSampleRateMs;
   }
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),GetBuffer()->BufferSize(), 0);
   GetVizBuffer()->WriteChunk(secondChannel,GetBuffer()->BufferSize(), 1);
   
   GetBuffer()->Reset();
}

void Panner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPanSlider->Draw();
   mWidenSlider->Draw();
   
   GetLeftPanGain(mPan);
   GetRightPanGain(mPan);
}

void Panner::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Panner::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void Panner::ButtonClicked(ClickButton *button)
{
}

void Panner::CheckboxUpdated(Checkbox* checkbox)
{
}

void Panner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("pan", moduleInfo, 0, mPanSlider);
   
   SetUpFromSaveData();
}

void Panner::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetPan(mModuleSaveData.GetFloat("pan"));
}
