//
//  ModWheel.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModWheel.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModWheel::ModWheel()
: mModWheel(0)
, mModWheelSlider(nullptr)
, mModulation(true)
{
   TheTransport->AddAudioPoller(this);
}

ModWheel::~ModWheel()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModWheel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mModWheelSlider = new FloatSlider(this,"modwheel",5,2,110,15,&mModWheel,-1,1);
}

void ModWheel::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mModWheelSlider->Draw();
}

void ModWheel::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      mModulation.GetModWheel(voiceIdx)->AppendTo(modulation.modWheel);
      modulation.modWheel = mModulation.GetModWheel(voiceIdx);
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void ModWheel::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void ModWheel::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mModWheelSlider)
      mModulation.GetModWheel(-1)->SetValue(mModWheel);
}

void ModWheel::CheckboxUpdated(Checkbox* checkbox)
{
}

void ModWheel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModWheel::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
