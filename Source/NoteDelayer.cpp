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
#include "Profiler.h"

NoteDelayer::NoteDelayer()
: mDelay(.25f)
, mDelaySlider(nullptr)
, mConsumeIndex(0)
, mAppendIndex(0)
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
   
   mDelaySlider = new FloatSlider(this,"delay",4,4,100,15,&mDelay,0,1,4);
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
      mNoteOutput.Flush(gTime);
}

void NoteDelayer::OnTransportAdvanced(float amount)
{
   PROFILER(NoteDelayer);
   
   ComputeSliders(0);
   
   int end = mAppendIndex;
   if (mAppendIndex < mConsumeIndex)
      end += kQueueSize;
   for (int i=mConsumeIndex; i<end; ++i)
   {
      const NoteInfo& info = mInputNotes[i % kQueueSize];
      if (gTime > info.mTriggerTime)
      {
         PlayNoteOutput(info.mTriggerTime, info.mPitch, info.mVelocity, -1, info.mModulation);
         mConsumeIndex = (mConsumeIndex + 1) % kQueueSize;
      }
   }
}

void NoteDelayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;
   
   if (velocity > 0)
      mLastNoteOnTime = time;
   
   if ((mAppendIndex + 1) % kQueueSize != mConsumeIndex)
   {
      NoteInfo info;
      info.mPitch = pitch;
      info.mVelocity = velocity;
      info.mTriggerTime = time + mDelay / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      info.mModulation = modulation;
      mInputNotes[mAppendIndex] = info;
      mAppendIndex = (mAppendIndex + 1) % kQueueSize;
   }
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
