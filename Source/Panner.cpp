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
, mPanSlider(NULL)
, mWiden(0)
, mWidenSlider(NULL)
, mWidenerBuffer(2048)
{
   mPanBuffer = new float[gBufferSize];
}

void Panner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this,"pan",5,20,110,15,&mPan,-1,1);
   mWidenSlider = new FloatSlider(this,"widen",50,2,65,15,&mWiden,-150,150,0);
}

Panner::~Panner()
{
   delete[] mPanBuffer;
}

void Panner::Process(double time)
{
   Profiler profiler("Panner");
   
   if (!mEnabled || GetTarget() == NULL)
      return;
   
   SyncBuffers(2);
   
   ChannelBuffer* out = GetTarget()->GetBuffer();
   
   if (abs(mWiden) > 0)
   {
      mWidenerBuffer.WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);
      mWidenerBuffer.ReadChunk(mPanBuffer, GetBuffer()->BufferSize(), abs(mWiden), 0);
   }
   else
   {
      BufferCopy(mPanBuffer, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
   }
   
   float* input1;
   float* input2;
   if (mWiden <= 0)
   {
      input1 = GetBuffer()->GetChannel(0);
      input2 = mPanBuffer;
   }
   else
   {
      input1 = mPanBuffer;
      input2 = GetBuffer()->GetChannel(0);
   }
   
   for (int i=0; i<GetBuffer()->BufferSize(); ++i)
   {
      ComputeSliders(i);
      mPanRamp.Start(mPan,2);
      float pan = mPanRamp.Value(time);
      
      input1[i] *= ofClamp(1-pan,0,1);
      out->GetChannel(0)[i] += input1[i];
      
      input2[i] *= ofClamp(1+pan,0,1);
      out->GetChannel(1)[i] += input2[i];
      
      
      time += gInvSampleRateMs;
   }
   
   GetVizBuffer()->WriteChunk(input1,GetBuffer()->BufferSize(), 0);
   GetVizBuffer()->WriteChunk(input2,GetBuffer()->BufferSize(), 1);
   
   GetBuffer()->Clear();
}

void Panner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPanSlider->Draw();
   mWidenSlider->Draw();
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
