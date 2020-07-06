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
, mGainSlider(nullptr)
{
   mModulationBuffer = new float[gBufferSize];
}

void AudioToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGainSlider = new FloatSlider(this, "gain", 3, 2, 100, 15, &mGain, 1, 10);
   mMinSlider = new FloatSlider(this, "min", mGainSlider, kAnchor_Below, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
   
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
   
   ofPushStyle();
   ofSetColor(0,255,0,gModuleDrawAlpha);
   ofBeginShape();
   float x,y;
   float w,h;
   mGainSlider->GetPosition(x, y, K(local));
   mGainSlider->GetDimensions(w, h);
   for (int i=0; i<gBufferSize; ++i)
   {
      ofVertex(ofMap(mModulationBuffer[i], -1, 1, x, x+w, K(clamp)), ofMap(i, 0, gBufferSize, y, y+h), K(clamp));
   }
   ofEndShape();
   ofPopStyle();
}

void AudioToCV::Process(double time)
{
   PROFILER(AudioToCV);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   assert(GetBuffer()->BufferSize());
   BufferCopy(mModulationBuffer, GetBuffer()->GetChannel(0), gBufferSize);
   Mult(mModulationBuffer, mGain, gBufferSize);
   
   GetBuffer()->Reset();
}

void AudioToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float AudioToCV::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn] / 2 + .5f, 0, 1, GetMin(), GetMax(), K(clamp));
}

void AudioToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
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
