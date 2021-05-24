//
//  NoteSustain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#include "NoteSustain.h"
#include "SynthGlobals.h"

NoteSustain::NoteSustain()
: mSustain(.25f)
, mSustainSlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

void NoteSustain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mSustainSlider = new FloatSlider(this,"duration",5,2,100,15,&mSustain,0.01f,4,4);
   mSustainSlider->SetMode(FloatSlider::kSquare);
}

NoteSustain::~NoteSustain()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteSustain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mSustainSlider->Draw();
}

void NoteSustain::OnTransportAdvanced(float amount)
{
   for (auto iter = mNoteOffs.begin(); iter != mNoteOffs.end();)
   {
      if (iter->mTime < gTime)
      {
         PlayNoteOutput(gTime, iter->mPitch, 0, iter->mVoiceIdx);
         iter = mNoteOffs.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

void NoteSustain::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (velocity > 0)
   {
      ComputeSliders(0);
      
      float durationMs = mSustain / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      
      bool found = false;
      for (auto& queued : mNoteOffs)
      {
         if (queued.mPitch == pitch)
         {
            queued.mTime = time + durationMs;
            queued.mVoiceIdx = voiceIdx;
            found = true;
            break;
         }
      }
      
      if (!found)
         mNoteOffs.push_back(QueuedNoteOff(time+durationMs, pitch, voiceIdx));
   }
}

void NoteSustain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteSustain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteSustain::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
