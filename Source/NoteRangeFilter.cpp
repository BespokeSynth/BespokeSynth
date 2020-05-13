/*
  ==============================================================================

    NoteRangeFilter.cpp
    Created: 29 Jan 2020 9:18:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteRangeFilter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

NoteRangeFilter::NoteRangeFilter()
: mMinPitch(24)
, mMinPitchSlider(nullptr)
, mMaxPitch(36)
, mMaxPitchSlider(nullptr)
{
   SetEnabled(true);
}

void NoteRangeFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinPitchSlider = new IntSlider(this,"min",5,2,80,15,&mMinPitch,0,127);
   mMaxPitchSlider = new IntSlider(this,"max",5,20,80,15,&mMaxPitch,0,127);
}

void NoteRangeFilter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mMinPitchSlider->Draw();
   mMaxPitchSlider->Draw();
}

void NoteRangeFilter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteRangeFilter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mMinPitchSlider || slider == mMaxPitchSlider)
      mNoteOutput.Flush(gTime);
}

void NoteRangeFilter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);
   
   if (!mEnabled || (pitch >= mMinPitch && pitch <= mMaxPitch))
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void NoteRangeFilter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteRangeFilter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
