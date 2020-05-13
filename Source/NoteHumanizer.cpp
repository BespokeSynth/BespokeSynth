/*
  ==============================================================================

    NoteHumanizer.cpp
    Created: 2 Nov 2016 7:56:21pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteHumanizer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NoteHumanizer::NoteHumanizer()
: mTime(33.0f)
, mTimeSlider(nullptr)
, mVelocity(.1f)
, mVelocitySlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

NoteHumanizer::~NoteHumanizer()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteHumanizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mTimeSlider = new FloatSlider(this,"time",4,4,100,15,&mTime,0,500);
   mVelocitySlider = new FloatSlider(this,"velocity",mTimeSlider,kAnchor_Below,100,15,&mVelocity,0,1);
   
   mTimeSlider->SetMode(FloatSlider::kSquare);
}

void NoteHumanizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mTimeSlider->Draw();
   mVelocitySlider->Draw();
}

void NoteHumanizer::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteHumanizer::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
   
   mNoteMutex.lock();
   for (auto iter = mInputNotes.begin(); iter != mInputNotes.end(); )
   {
      const NoteInfo& info = *iter;
      if (gTime > info.mTriggerTime)
      {
         PlayNoteOutput(info.mTriggerTime, info.mPitch, info.mVelocity, -1, info.mModulation);
         
         iter = mInputNotes.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
   mNoteMutex.unlock();
}

void NoteHumanizer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity);
      return;
   }
   
   NoteInfo info;
   info.mPitch = pitch;
   if (velocity > 0)
      info.mVelocity = ofClamp((velocity/127.0f * ofRandom(1 - mVelocity, 1 + mVelocity)) * 127, 1, 127);
   else
      info.mVelocity = 0;
   info.mTriggerTime = time + ofRandom(0, mTime);
   info.mModulation = modulation;
   
   mNoteMutex.lock();
   mInputNotes.push_back(info);
   mNoteMutex.unlock();
}

void NoteHumanizer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteHumanizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteHumanizer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
