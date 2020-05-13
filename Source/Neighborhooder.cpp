//
//  Neighborhooder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/10/13.
//
//

#include "Neighborhooder.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

Neighborhooder::Neighborhooder()
: mMinPitch(55)
, mPitchRange(16)
{
}

void Neighborhooder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinSlider = new IntSlider(this,"min",4,3,84,15,&mMinPitch,0,127);
   mRangeSlider = new IntSlider(this,"range",mMinSlider,kAnchor_Below,116,15,&mPitchRange,12,36);
}

void Neighborhooder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mRangeSlider->Draw();
   
   DrawTextNormal(NoteName(mMinPitch)+ofToString(mMinPitch/12 - 2), 91, 15);
}

void Neighborhooder::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void Neighborhooder::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mMinSlider || slider == mRangeSlider)
      mNoteOutput.Flush(gTime);
}

void Neighborhooder::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   while (pitch >= mMinPitch + mPitchRange)
      pitch -= TheScale->GetTet();
   while (pitch < mMinPitch)
      pitch += TheScale->GetTet();
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void Neighborhooder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Neighborhooder::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
