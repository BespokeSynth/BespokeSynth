//
//  PitchAssigner.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/27/15.
//
//

#include "PitchSetter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

PitchSetter::PitchSetter()
: mPitch(36)
, mPitchSlider(nullptr)
{
   SetEnabled(true);
}

void PitchSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchSlider = new IntSlider(this,"pitch",5,2,80,15,&mPitch,0,127);
}

void PitchSetter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mPitchSlider->Draw();
}

void PitchSetter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void PitchSetter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mPitchSlider)
      mNoteOutput.Flush(gTime);
}

void PitchSetter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);
   
   if (mEnabled)
      PlayNoteOutput(time, mPitch, velocity, voiceIdx, modulation);
   else
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void PitchSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchSetter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
