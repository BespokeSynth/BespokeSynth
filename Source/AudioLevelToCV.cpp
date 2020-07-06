/*
  ==============================================================================

    AudioLevelToCV.cpp
    Created: 9 Oct 2018 10:26:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioLevelToCV.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

AudioLevelToCV::AudioLevelToCV()
: IAudioProcessor(gBufferSize)
, mGain(1)
, mGainSlider(nullptr)
, mAttackSlider(nullptr)
, mReleaseSlider(nullptr)
, mVal(0)
, mAttack(10)
, mRelease(10)
{
   mModulationBuffer = new float[gBufferSize];
}

void AudioLevelToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGainSlider = new FloatSlider(this, "gain", 3, 2, 100, 15, &mGain, 1, 100);
   mAttackSlider = new FloatSlider(this, "attack", mGainSlider, kAnchor_Below, 100, 15, &mAttack, .01f, 1000);
   mReleaseSlider = new FloatSlider(this, "release", mAttackSlider, kAnchor_Below, 100, 15, &mRelease, .01f, 1000);
   mMinSlider = new FloatSlider(this, "min", mReleaseSlider, kAnchor_Below, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
   
   mGainSlider->SetMode(FloatSlider::kSquare);
   mAttackSlider->SetMode(FloatSlider::kSquare);
   mReleaseSlider->SetMode(FloatSlider::kSquare);
   
   //update mAttackFactor and mReleaseFactor
   FloatSliderUpdated(mAttackSlider, 0);
   FloatSliderUpdated(mReleaseSlider, 0);
   
   GetPatchCableSource()->SetEnabled(false);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

AudioLevelToCV::~AudioLevelToCV()
{
   delete[] mModulationBuffer;
}

void AudioLevelToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGainSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();
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
      ofVertex(ofMap(mModulationBuffer[i], 0, 1, x, x+w, K(clamp)), ofMap(i, 0, gBufferSize, y, y+h), K(clamp));
   }
   ofEndShape();
   ofPopStyle();
}

void AudioLevelToCV::Process(double time)
{
   PROFILER(AudioLevelToCV);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   assert(GetBuffer()->BufferSize());
   float* data = GetBuffer()->GetChannel(0);
   for (int i=0; i<gBufferSize; ++i)
   {
      float sample = fabsf(data[i]);
      if (sample > mVal)
         mVal = mAttackFactor * (mVal - sample) + sample;
      else
         mVal = mReleaseFactor * (mVal - sample) + sample;
      mModulationBuffer[i] = mVal * mGain;
   }
   
   GetBuffer()->Reset();
}

void AudioLevelToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float AudioLevelToCV::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn], 0, 1, GetMin(), GetMax(), K(clamp));
}

void AudioLevelToCV::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mAttackSlider)
      mAttackFactor = powf(.01f, 1.0f / (mAttack * gSampleRateMs));
   if (slider == mReleaseSlider)
      mReleaseFactor = powf(.01f, 1.0f / (mRelease * gSampleRateMs));
}

void AudioLevelToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void AudioLevelToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void AudioLevelToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}
