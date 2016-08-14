//
//  Amplifier.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 7/13/13.
//
//

#include "Amplifier.h"
#include "ModularSynth.h"
#include "Profiler.h"

Amplifier::Amplifier()
: mBoost(0)
, mBoostSlider(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

void Amplifier::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mBoostSlider = new FloatSlider(this,"boost",5,2,110,15,&mBoost,1,4);
}

Amplifier::~Amplifier()
{
   delete[] mInputBuffer;
}

float* Amplifier::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void Amplifier::Process(double time)
{
   Profiler profiler("Amplifier");

   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = gBufferSize;
   if (GetTarget())
   {
      float* out = GetTarget()->GetBuffer(bufferSize);
      assert(bufferSize == gBufferSize);
      
      Mult(mInputBuffer, mBoost*mBoost, bufferSize);
      Add(out, mInputBuffer, bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
}

void Amplifier::DrawModule()
{

   
   if (Minimized() || IsVisible() == false)
      return;
   
   mBoostSlider->Draw();
}

void Amplifier::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("boost", moduleInfo, 1, mBoostSlider);

   SetUpFromSaveData();
}

void Amplifier::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetBoost(mModuleSaveData.GetFloat("boost"));
}



