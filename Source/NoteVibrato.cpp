//
//  NoteVibrato.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#include "NoteVibrato.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteVibrato::NoteVibrato()
: mVibratoInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mVibratoAmount(0)
, mVibratoSlider(nullptr)
, mModulation(true)
{
   TheTransport->AddAudioPoller(this);
}

NoteVibrato::~NoteVibrato()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteVibrato::CreateUIControls()
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

void NoteVibrato::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mVibratoSlider->Draw();
   mIntervalSelector->Draw();
}

void NoteVibrato::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void NoteVibrato::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(voiceIdx)->AppendTo(modulation.pitchBend);
      modulation.pitchBend = mModulation.GetPitchBend(voiceIdx);
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteVibrato::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mVibratoSlider)
      mModulation.GetPitchBend(-1)->SetLFO(mVibratoInterval, mVibratoAmount);
}

void NoteVibrato::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      mModulation.GetPitchBend(-1)->SetLFO(mVibratoInterval, mVibratoAmount);
}

void NoteVibrato::CheckboxUpdated(Checkbox* checkbox)
{
}

void NoteVibrato::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("range", moduleInfo, 1, 0, 64, K(isTextField));
   
   SetUpFromSaveData();
}

void NoteVibrato::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVibratoSlider->SetExtents(0, mModuleSaveData.GetFloat("range"));
}

