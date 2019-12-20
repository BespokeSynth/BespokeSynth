//
//  NoteDelayer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#include "NoteDelayer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NoteDelayer::NoteDelayer()
: mDelay(.25f)
, mDelaySlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

NoteDelayer::~NoteDelayer()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteDelayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mDelaySlider = new FloatSlider(this,"delay",4,4,100,15,&mDelay,0,1);
}

void NoteDelayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mDelaySlider->Draw();
   
   float t = (gTime - mLastNoteOnTime) / (mDelay * TheTransport->GetDuration(kInterval_1n));
   if (t > 0 && t < 1)
   {
      ofPushStyle();
      ofNoFill();
      ofCircle(54, 11, 10);
      ofFill();
      ofSetColor(255,255,255,gModuleDrawAlpha);
      ofCircle(54 + sin(t * TWO_PI) * 10, 11 - cos(t * TWO_PI) * 10, 2);
      ofPopStyle();
   }
}

void NoteDelayer::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void NoteDelayer::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
   
   mNoteMutex.lock();
   vector<NoteInfo> notesToPlay;
   for (auto iter = mInputNotes.begin(); iter != mInputNotes.end(); )
   {
      const NoteInfo& info = *iter;
      if (gTime > info.mTriggerTime)
      {
         notesToPlay.push_back(info);
         iter = mInputNotes.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
   for (auto info : notesToPlay)
      PlayNoteOutput(info.mTriggerTime, info.mPitch, info.mVelocity, -1, info.mModulation);
   mNoteMutex.unlock();
}

void NoteDelayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;
   
   if (velocity > 0)
      mLastNoteOnTime = time;
   
   NoteInfo info;
   info.mPitch = pitch;
   info.mVelocity = velocity;
   info.mTriggerTime = time + mDelay * TheTransport->GetDuration(kInterval_1n);
   info.mModulation = modulation;

   mNoteMutex.lock();
   mInputNotes.push_back(info);
   mNoteMutex.unlock();
}

void NoteDelayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteDelayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteDelayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
