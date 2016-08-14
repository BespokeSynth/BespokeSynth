//
//  Stereofier.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/18/12.
//
//

#include "Stereofier.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Stereofier::Stereofier()
: mPan(0)
, mPanSlider(NULL)
, mWiden(0)
, mWidenSlider(NULL)
, mVizBuffer2(VIZ_BUFFER_SECONDS*gSampleRate)
, mWidenerBuffer(2048)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   mInputBuffer2 = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

void Stereofier::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this,"pan",5,20,110,15,&mPan,-1,1);
   mWidenSlider = new FloatSlider(this,"widen",50,2,65,15,&mWiden,-150,150,0);
   
   int w,h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w/2-10,h+3);
   
   mPatchCableSource2 = new PatchCableSource(this, kConnectionType_Audio);
   mPatchCableSource2->SetManualPosition(w/2+10,h+3);
   mPatchCableSource2->SetOverrideVizBuffer(&mVizBuffer2);
   AddPatchCableSource(mPatchCableSource2);
}

Stereofier::~Stereofier()
{
   delete[] mInputBuffer;
}

float* Stereofier::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void Stereofier::Process(double time)
{
   Profiler profiler("Stereofier");

   if (!mEnabled || GetTarget() == NULL || GetTarget2() == NULL)
      return;
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   float* out2 = GetTarget2()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   if (abs(mWiden) > 0)
   {
      mWidenerBuffer.WriteChunk(mInputBuffer, bufferSize);
      mWidenerBuffer.ReadChunk(mInputBuffer2, bufferSize, abs(mWiden));
   }
   else
   {
      memcpy(mInputBuffer2, mInputBuffer, sizeof(float)*gBufferSize);
   }
   
   float* input1;
   float* input2;
   if (mWiden <= 0)
   {
      input1 = mInputBuffer;
      input2 = mInputBuffer2;
   }
   else
   {
      input1 = mInputBuffer2;
      input2 = mInputBuffer;
   }
   
   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);
      mPanRamp.Start(mPan,2);
      float pan = mPanRamp.Value(time);
      
      if (GetTarget())
      {
         input1[i] *= ofClamp(1-pan,0,1);
         out[i] += input1[i];
      }
      
      if (GetTarget2())
      {
         input2[i] *= ofClamp(1+pan,0,1);
         out2[i] += input2[i];
      }
      
      time += gInvSampleRateMs;
   }

   GetVizBuffer()->WriteChunk(input1,bufferSize);
   mVizBuffer2.WriteChunk(input2, bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
}

void Stereofier::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPanSlider->Draw();
   mWidenSlider->Draw();
}

void Stereofier::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Stereofier::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void Stereofier::ButtonClicked(ClickButton *button)
{
}

void Stereofier::CheckboxUpdated(Checkbox* checkbox)
{
}

void Stereofier::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("target2", moduleInfo);
   mModuleSaveData.LoadFloat("pan", moduleInfo, 0, mPanSlider);

   SetUpFromSaveData();
}

void Stereofier::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IClickable* target2 = TheSynth->FindModule(mModuleSaveData.GetString("target2"));
   if (target2)
      mPatchCableSource2->AddPatchCable(target2);
   SetPan(mModuleSaveData.GetFloat("pan"));
}


