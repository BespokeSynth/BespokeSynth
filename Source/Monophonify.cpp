//
//  Monophonify.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#include "Monophonify.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

Monophonify::Monophonify()
: mGlideTime(0)
, mGlideSlider(nullptr)
{
}

void Monophonify::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGlideSlider = new FloatSlider(this,"glide",4,2,100,15,&mGlideTime,0,1000);
   
   mGlideSlider->SetMode(FloatSlider::kSquare);
}

void Monophonify::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mGlideSlider->Draw();
}

void Monophonify::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      return;
   }
   
   mPitchBend.AppendTo(pitchBend);

   voiceIdx = 0;
   
   mHeldNotesMutex.lock();
   if (velocity > 0)
   {
      if (mHeldNotes.size())
      {
         mPitchBend.RampValue(mHeldNotes.rbegin()->mPitch - pitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
         PlayNoteOutput(time, pitch, velocity, voiceIdx, &mPitchBend, modWheel, pressure);
         for (list<HeldNote>::iterator iter = mHeldNotes.begin(); iter != mHeldNotes.end(); ++iter)
            PlayNoteOutput(gTime,(*iter).mPitch,0,-1, &mPitchBend, modWheel, pressure);
      }
      else
      {
         mNoteOutput.Flush();
         PlayNoteOutput(time, pitch, velocity, voiceIdx, &mPitchBend, modWheel, pressure);
         mPitchBend.SetValue(0);
      }
      mHeldNotes.push_back(HeldNote(pitch, velocity));
   }
   else
   {
      bool wasCurrNote = mHeldNotes.size() && mHeldNotes.rbegin()->mPitch == pitch;
      
      for (list<HeldNote>::iterator i = mHeldNotes.begin(); i!=mHeldNotes.end(); ++i)
      {
         if ((*i).mPitch == pitch)
            i = mHeldNotes.erase(i);
      }
      
      if (mHeldNotes.size() && wasCurrNote)
      {
         HeldNote heldNote = *(mHeldNotes.rbegin());

         mPitchBend.RampValue(pitch - heldNote.mPitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
         PlayNoteOutput(time, heldNote.mPitch, heldNote.mVelocity, voiceIdx, &mPitchBend, modWheel, pressure);
      }

      PlayNoteOutput(time, pitch, 0, voiceIdx, &mPitchBend, modWheel, pressure);
   }
   mHeldNotesMutex.unlock();
}

void Monophonify::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush();
      mHeldNotesMutex.lock();
      mHeldNotes.clear();
      mHeldNotesMutex.unlock();
   }
}

void Monophonify::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Monophonify::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Monophonify::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

