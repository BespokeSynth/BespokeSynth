//
//  NoteFilter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#include "NoteFilter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

NoteFilter::NoteFilter()
: mMinPitch(24)
, mMinPitchSlider(nullptr)
, mMaxPitch(36)
, mMaxPitchSlider(nullptr)
{
   SetEnabled(true);
}

void NoteFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinPitchSlider = new IntSlider(this,"min",5,2,80,15,&mMinPitch,0,127);
   mMaxPitchSlider = new IntSlider(this,"max",5,20,80,15,&mMaxPitch,0,127);
}

void NoteFilter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mMinPitchSlider->Draw();
   mMaxPitchSlider->Draw();
}

void NoteFilter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void NoteFilter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mMinPitchSlider || slider == mMaxPitchSlider)
      mNoteOutput.Flush();
}

void NoteFilter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);
   
   if (!mEnabled || (pitch >= mMinPitch && pitch <= mMaxPitch))
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void NoteFilter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteFilter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
