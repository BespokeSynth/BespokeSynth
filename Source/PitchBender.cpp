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
, mRange(2)
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
   mBendSlider = new FloatSlider(this,"bend",5,2,110,15,&mBend,-mRange,mRange);
}

void PitchBender::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mBendSlider->Draw();
}

void PitchBender::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(voiceIdx)->AppendTo(modulation.pitchBend);
      modulation.pitchBend = mModulation.GetPitchBend(voiceIdx);
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
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
   mModuleSaveData.LoadFloat("range", moduleInfo, 2, 0, 48, true);
   
   SetUpFromSaveData();
}

void PitchBender::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mRange = mModuleSaveData.GetFloat("range");
   mBendSlider->SetExtents(-mRange, mRange);
}


