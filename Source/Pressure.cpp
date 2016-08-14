//
//  Pressure.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "Pressure.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

Pressure::Pressure()
: mPressure(0)
, mPressureSlider(NULL)
, mModulation(true)
{
   TheTransport->AddAudioPoller(this);
}

Pressure::~Pressure()
{
   TheTransport->RemoveAudioPoller(this);
}

void Pressure::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPressureSlider = new FloatSlider(this,"pressure",5,2,110,15,&mPressure,0,1);
}

void Pressure::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mPressureSlider->Draw();
}

void Pressure::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mEnabled)
   {
      mModulation.GetPressure(voiceIdx)->AppendTo(pressure);
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, mModulation.GetPressure(voiceIdx));
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void Pressure::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void Pressure::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mPressureSlider)
      mModulation.GetPressure(-1)->SetValue(mPressure);
}

void Pressure::CheckboxUpdated(Checkbox* checkbox)
{
}

void Pressure::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void Pressure::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
