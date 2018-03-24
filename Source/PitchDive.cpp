//
//  PitchDive.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#include "PitchDive.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

PitchDive::PitchDive()
: mStart(0)
, mStartSlider(nullptr)
, mTime(0)
, mTimeSlider(nullptr)
, mModulation(false)
{
}

PitchDive::~PitchDive()
{
}

void PitchDive::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mStartSlider = new FloatSlider(this,"start",5,2,110,15,&mStart,-3,3);
   mTimeSlider = new FloatSlider(this,"time",5,20,110,15,&mTime,0,1000);
   
   mTimeSlider->SetMode(FloatSlider::kSquare);
}

void PitchDive::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mStartSlider->Draw();
   mTimeSlider->Draw();
}

void PitchDive::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0 && mStart != 0 && mTime != 0)
   {
      ComputeSliders(0);
      mModulation.GetPitchBend(voiceIdx)->AppendTo(modulation.pitchBend);
      mModulation.GetPitchBend(voiceIdx)->RampValue(mStart, 0, mTime);
      modulation.pitchBend = mModulation.GetPitchBend(voiceIdx);
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void PitchDive::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void PitchDive::CheckboxUpdated(Checkbox* checkbox)
{
}

void PitchDive::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchDive::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
