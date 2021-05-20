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
#include "UIControlMacros.h"

NoteRangeFilter::NoteRangeFilter()
: mMinPitch(24)
, mMinPitchSlider(nullptr)
, mMaxPitch(36)
, mMaxPitchSlider(nullptr)
, mWrap(false)
{
   SetEnabled(true);
}

void NoteRangeFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   INTSLIDER(mMinPitchSlider,"min",&mMinPitch,0,127);
   INTSLIDER(mMaxPitchSlider,"max",&mMaxPitch,0,127);
   CHECKBOX(mWrapCheckbox,"wrap",&mWrap);
   ENDUIBLOCK(mWidth, mHeight);
}

void NoteRangeFilter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mMinPitchSlider->Draw();
   mMaxPitchSlider->Draw();
   mWrapCheckbox->Draw();
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

   if (mWrap && mMaxPitch > mMinPitch)
   {
      int length = mMaxPitch - mMinPitch + 1;
      while (pitch < mMinPitch)
         pitch += length;
      while (pitch > mMaxPitch)
         pitch -= length;
   }
   
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
