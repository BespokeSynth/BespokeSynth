/*
  ==============================================================================

    AudioToPulse.cpp
    Created: 31 Mar 2021 10:18:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioToPulse.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Profiler.h"
#include "UIControlMacros.h"

AudioToPulse::AudioToPulse()
: IAudioProcessor(gBufferSize)
, mThresholdSlider(nullptr)
, mReleaseSlider(nullptr)
, mVal(0)
, mThreshold(.5f)
, mRelease(750)
{
}

AudioToPulse::~AudioToPulse()
{
}

void AudioToPulse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mThresholdSlider, "threshold", &mThreshold, 0, 1);
   FLOATSLIDER(mReleaseSlider, "release", &mRelease, .01f, 1000);
   ENDUIBLOCK(mWidth, mHeight);
   
   mReleaseSlider->SetMode(FloatSlider::kSquare);

   //update mReleaseFactor
   FloatSliderUpdated(mReleaseSlider, 0);
}

void AudioToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mThresholdSlider->Draw();
   mReleaseSlider->Draw();

   ofPushStyle();
   ofFill();
   ofSetColor(255, 0, 0, gModuleDrawAlpha*.4f);
   ofRectangle rect = mThresholdSlider->GetRect(true);
   rect.width *= ofClamp(mVal, 0, 1);
   ofRect(rect);
   ofPopStyle();
}

void AudioToPulse::Process(double time)
{
   PROFILER(AudioToPulse);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   const float kAttackTimeMs = 5;
   const float kAttackFactor = powf(.01f, 1.0f / (kAttackTimeMs * gSampleRateMs));

   assert(GetBuffer()->BufferSize());
   Clear(gWorkBuffer, gBufferSize);
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      Add(gWorkBuffer, GetBuffer()->GetChannel(ch), gBufferSize);
   Mult(gWorkBuffer, 1.0f / GetBuffer()->NumActiveChannels(), gBufferSize);
   for (int i = 0; i < gBufferSize; ++i)
   {
      float sample = fabsf(gWorkBuffer[i]);
      if (sample > mVal)
      {
         float oldVal = mVal;
         mVal = kAttackFactor * (mVal - sample) + sample;
         if (mVal >= mThreshold && oldVal < mThreshold)
            DispatchPulse(GetPatchCableSource(), time + i * gInvSampleRateMs, 1, 0);
      }
      else
      {
         mVal = mReleaseFactor * (mVal - sample) + sample;
      }
   }

   GetBuffer()->Reset();
}

void AudioToPulse::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mReleaseSlider)
      mReleaseFactor = powf(.01f, 1.0f / (mRelease * gSampleRateMs));
}

void AudioToPulse::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void AudioToPulse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void AudioToPulse::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
