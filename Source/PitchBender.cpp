//
//  PitchBender.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 9/7/14.
//
//

#include "PitchBender.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

PitchBender::PitchBender()
: mBend(0)
, mBendSlider(nullptr)
//, mBendingCheckbox(this,"bending",HIDDEN_UICONTROL,HIDDEN_UICONTROL,mBendSlider->mTouching)
, mModulation(true)
{
   //mBendSlider->SetRelative(true);
   TheTransport->AddAudioPoller(this);
}

PitchBender::~PitchBender()
{
   TheTransport->RemoveAudioPoller(this);
}

void PitchBender::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mBendSlider = new FloatSlider(this,"bend",5,2,110,15,&mBend,-2,2);
}

void PitchBender::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mBendSlider->Draw();
}

void PitchBender::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(voiceIdx)->AppendTo(pitchBend);
      PlayNoteOutput(time, pitch, velocity, voiceIdx, mModulation.GetPitchBend(voiceIdx), modWheel, pressure);
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void PitchBender::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void PitchBender::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mBendSlider)
      mModulation.GetPitchBend(-1)->SetValue(mBend);
}

void PitchBender::CheckboxUpdated(Checkbox* checkbox)
{
   /*if (checkbox == &mBendingCheckbox)
   {
      mBendSlider->UpdateTouching();
   }*/
}

void PitchBender::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchBender::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


