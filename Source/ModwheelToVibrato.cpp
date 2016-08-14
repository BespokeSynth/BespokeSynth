//
//  ModwheelToVibrato.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModwheelToVibrato.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModwheelToVibrato::ModwheelToVibrato()
: mVibratoInterval(kInterval_16n)
, mIntervalSelector(NULL)
, mVibratoAmount(1)
, mVibratoSlider(NULL)
, mModulation(true)
{
}

ModwheelToVibrato::~ModwheelToVibrato()
{
}

void ModwheelToVibrato::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVibratoSlider = new FloatSlider(this,"vibrato",3,3,90,15,&mVibratoAmount,0,1);
   mIntervalSelector = new DropdownList(this,"vibinterval",96,3,(int*)(&mVibratoInterval));
   
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

void ModwheelToVibrato::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mVibratoSlider->Draw();
   mIntervalSelector->Draw();
}

void ModwheelToVibrato::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(voiceIdx)->AppendTo(pitchBend);
      mModulation.GetPitchBend(voiceIdx)->SetLFO(mVibratoInterval, mVibratoAmount);
      mModulation.GetPitchBend(voiceIdx)->MultiplyIn(modWheel);
      PlayNoteOutput(time, pitch, velocity, voiceIdx, mModulation.GetPitchBend(voiceIdx), NULL, pressure);
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void ModwheelToVibrato::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void ModwheelToVibrato::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void ModwheelToVibrato::CheckboxUpdated(Checkbox* checkbox)
{
}

void ModwheelToVibrato::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModwheelToVibrato::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

