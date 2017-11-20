/*
  ==============================================================================

    AudioToCV.cpp
    Created: 18 Nov 2017 10:46:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioToCV.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

AudioToCV::AudioToCV()
: IAudioProcessor(gBufferSize)
, mGain(1)
, mMin(0)
, mMax(1)
, mTargetCable(nullptr)
, mTarget(nullptr)
, mGainSlider(nullptr)
, mMinSlider(nullptr)
, mMaxSlider(nullptr)
{
   mModulationBuffer = new float[gBufferSize];
}

void AudioToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGainSlider = new FloatSlider(this, "gain", 3, 2, 100, 15, &mGain, 1, 5);
   mMinSlider = new FloatSlider(this, "min", mGainSlider, kAnchor_Below, 100, 15, &mMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mMax, 0, 1);
   
   GetPatchCableSource()->SetEnabled(false);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

AudioToCV::~AudioToCV()
{
   delete[] mModulationBuffer;
}

void AudioToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGainSlider->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   
   ofBeginShape();
   float x,y;
   int w,h;
   mGainSlider->GetPosition(x, y, K(local));
   mGainSlider->GetDimensions(w, h);
   for (int i=0; i<gBufferSize; ++i)
   {
      ofVertex(ofMap(mModulationBuffer[i], -1, 1, x, x+w, K(clamp)), ofMap(i, 0, gBufferSize, y, y+h), K(clamp));
   }
   ofEndShape();
}

void AudioToCV::Process(double time)
{
   Profiler profiler("AudioToCV");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   assert(GetBuffer()->BufferSize());
   BufferCopy(mModulationBuffer, GetBuffer()->GetChannel(0), gBufferSize);
   Mult(mModulationBuffer, mGain, gBufferSize);
   
   GetBuffer()->Reset();
}

void AudioToCV::PostRepatch(PatchCableSource* cableSource)
{
   if (mTarget != nullptr)
      mTarget->SetModulator(nullptr);
   
   if (mTargetCable->GetPatchCables().empty() == false)
   {
      mTarget = dynamic_cast<FloatSlider*>(mTargetCable->GetPatchCables()[0]->GetTarget());
      mTarget->SetModulator(this);
      mMin = mTarget->GetMin();
      mMax = mTarget->GetMax();
      mMinSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mMinSlider->SetMode(mTarget->GetMode());
      mMaxSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mMaxSlider->SetMode(mTarget->GetMode());
   }
   else
   {
      mTarget = nullptr;
   }
}

float AudioToCV::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn] / 2 + .5f, 0, 1, mMin, mMax, K(clamp));
}

void AudioToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void AudioToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}
