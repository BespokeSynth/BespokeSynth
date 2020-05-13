//
//  NoteCreator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#include "NoteCreator.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "PolyphonyMgr.h"

NoteCreator::NoteCreator()
: mTriggerButton(nullptr)
, mPitch(48)
, mVelocity(1)
, mDuration(100)
, mPitchEntry(nullptr)
, mVelocitySlider(nullptr)
, mNoteOn(false)
, mStartTime(0)
, mNoteOnCheckbox(nullptr)
, mNoteOnByTrigger(false)
, mVoiceIndex(-1)
{
   SetIsNoteOrigin(true);
   TheTransport->AddAudioPoller(this);
}

NoteCreator::~NoteCreator()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteCreator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchEntry = new TextEntry(this,"pitch",5,3,3,&mPitch,0,127);
   mTriggerButton = new ClickButton(this,"trigger",65,3);
   mNoteOnCheckbox = new Checkbox(this,"on",35,3,&mNoteOn);
   mVelocitySlider = new FloatSlider(this,"velocity",5,21,100,15,&mVelocity,0,1);
   mDurationSlider = new FloatSlider(this,"duration",5,39,100,15,&mDuration,1,1000);
}

void NoteCreator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPitchEntry->Draw();
   mNoteOnCheckbox->Draw();
   mTriggerButton->Draw();
   mVelocitySlider->Draw();
   mDurationSlider->Draw();
}

void NoteCreator::OnTransportAdvanced(float amount)
{
   if (mNoteOn && mNoteOnByTrigger && gTime > mStartTime + mDuration)
   {
      mNoteOn = false;
      mNoteOutput.Flush(gTime);
   }
}

void NoteCreator::OnPulse(double time, float velocity, int flags)
{
   TriggerNote(time, velocity * mVelocity);
}

void NoteCreator::TriggerNote(double time, float velocity)
{
   mNoteOn = true;
   mNoteOnByTrigger = true;
   mStartTime = time;
   PlayNoteOutput(mStartTime, mPitch, velocity*127);
}

void NoteCreator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
   if (checkbox == mNoteOnCheckbox)
   {
      if (mNoteOn)
      {
         mNoteOnByTrigger = false;
         PlayNoteOutput(gTime, mPitch, mVelocity*127, mVoiceIndex);
      }
      else
      {
         PlayNoteOutput(gTime, mPitch, 0, mVoiceIndex);
         mNoteOutput.Flush(gTime);
      }
   }
}

void NoteCreator::ButtonClicked(ClickButton* button)
{
   if (button == mTriggerButton)
      TriggerNote(gTime, mVelocity);
}

void NoteCreator::TextEntryComplete(TextEntry* entry)
{
   if (entry == mPitchEntry)
   {
      mNoteOn = false;
      mNoteOutput.Flush(gTime);
   }
}

void NoteCreator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voice index", moduleInfo, -1, -1, kNumVoices);
   
   SetUpFromSaveData();
}

void NoteCreator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVoiceIndex = mModuleSaveData.GetInt("voice index");
}
