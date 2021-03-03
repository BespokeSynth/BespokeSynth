/*
  ==============================================================================

    UnstablePitch.cpp
    Created: 2 Mar 2021 7:48:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "UnstablePitch.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

UnstablePitch::UnstablePitch()
   : mPerlin(.2f, .1f, 0)
   , mModulation(false)
{
   TheTransport->AddAudioPoller(this);

   for (int voice = 0; voice < kNumVoices; ++voice)
      mModulation.GetPitchBend(voice)->CreateBuffer();
}

UnstablePitch::~UnstablePitch()
{
   TheTransport->RemoveAudioPoller(this);
}

void UnstablePitch::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(3, 40, 120);
   FLOATSLIDER(mAmountSlider, "amount", &mPerlin.mPerlinAmount, 0, 1);
   FLOATSLIDER(mWarbleSlider, "warble", &mPerlin.mPerlinWarble, 0, 1);
   FLOATSLIDER(mNoiseSlider, "noise", &mPerlin.mPerlinNoise, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mWarbleSlider->SetMode(FloatSlider::kSquare);
   mNoiseSlider->SetMode(FloatSlider::kSquare);
}

void UnstablePitch::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofRectangle rect(3, 3, mWidth-6, 34);
   double perlinTime = gTime;
   const int kGridSize = 30;
   ofFill();
   for (int col = 0; col < kGridSize; ++col)
   {
      float x = rect.x + col * (rect.width / kGridSize);
      float y = rect.y;
      float val = mPerlin.GetValue(gTime, x / rect.width * 10, 0) * ofClamp(mPerlin.mPerlinAmount * 5, 0, 1);
      ofSetColor(val * 255, 0, val * 255);
      ofRect(x, y, (rect.width / kGridSize) + .5f, rect.height + .5f, 0);
   }

   ofNoFill();
   ofSetColor(0, 255, 0, 90);
   for (int voice = 0; voice < kNumVoices; ++voice)
   {
      if (mIsVoiceUsed[voice])
      {
         ofBeginShape();
         for (int i = 0; i < gBufferSize; ++i)
         {
            float sample = ofClamp(mModulation.GetPitchBend(voice)->GetBufferValue(i) * 5, -1, 1);
            ofVertex((i*rect.width) / gBufferSize + rect.x, rect.y + (-sample * .5f + .5f) * rect.height);
         }
         ofEndShape();
      }
   }

   ofPopStyle();

   mAmountSlider->Draw();
   mWarbleSlider->Draw();
   mNoiseSlider->Draw();
}

void UnstablePitch::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      if (voiceIdx == -1)
      {
         if (velocity > 0)
         {
            for (size_t i = 0; i < mIsVoiceUsed.size(); ++i)
            {
               if (mIsVoiceUsed[i] == false)
               {
                  voiceIdx = i;
                  break;
               }
            }
         }
         else
         {
            voiceIdx = mPitchToVoice[pitch];
         }
      }

      if (voiceIdx == -1)
         voiceIdx = 0;

      mIsVoiceUsed[voiceIdx] = velocity > 0;
      mPitchToVoice[pitch] = (velocity > 0) ? voiceIdx : -1;
   
      mModulation.GetPitchBend(voiceIdx)->AppendTo(modulation.pitchBend);
      modulation.pitchBend = mModulation.GetPitchBend(voiceIdx);
   }

   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void UnstablePitch::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);

   for (int voice = 0; voice < kNumVoices; ++voice)
   {
      if (mIsVoiceUsed[voice])
      {
         for (int i = 0; i < gBufferSize; ++i)
            gWorkBuffer[i] = ofMap(mPerlin.GetValue(gTime + i * gInvSampleRateMs, (gTime + i * gInvSampleRateMs) / 1000, voice), 0, 1, -mPerlin.mPerlinAmount, mPerlin.mPerlinAmount);

         mModulation.GetPitchBend(voice)->FillBuffer(gWorkBuffer);
      }
   }
}

void UnstablePitch::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void UnstablePitch::CheckboxUpdated(Checkbox* checkbox)
{
}

void UnstablePitch::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void UnstablePitch::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


