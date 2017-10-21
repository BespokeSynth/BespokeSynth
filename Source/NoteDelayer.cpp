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
   for (auto iter = mInputNotes.begin(); iter != mInputNotes.end(); )
   {
      const NoteInfo& info = *iter;
      if (gTime > info.mTriggerTime)
      {
         PlayNoteOutput(info.mTriggerTime, info.mPitch, info.mVelocity, -1, info.mPitchBend, info.mModWheel, info.mPressure);
      
         iter = mInputNotes.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
   mNoteMutex.unlock();
}

void NoteDelayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (!mEnabled)
      return;
   
   NoteInfo info;
   info.mPitch = pitch;
   info.mVelocity = velocity;
   info.mTriggerTime = time + mDelay * TheTransport->GetDuration(kInterval_1n);
   info.mPitchBend = pitchBend;
   info.mModWheel = modWheel;
   info.mPressure = pressure;

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
