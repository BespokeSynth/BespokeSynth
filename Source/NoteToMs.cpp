//
//  NoteToMs.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/2/16.
//
//

#include "NoteToMs.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToMs::NoteToMs()
: mControlCable(NULL)
, mPitch(0)
, mBend(0)
, mPitchBend(NULL)
{
   TheTransport->AddAudioPoller(this);
}

NoteToMs::~NoteToMs()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteToMs::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mControlCable);
}

void NoteToMs::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToMs::PostRepatch(PatchCableSource* cableSource)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void NoteToMs::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = pitchBend;
      if (mTarget)
         mTarget->SetValue(1000/TheScale->PitchToFreq(mPitch+mBend));
   }
}

void NoteToMs::OnTransportAdvanced(float amount)
{
   float bend = mPitchBend ? mPitchBend->GetValue(0) : 0;
   if (mEnabled && bend != mBend)
   {
      mBend = bend;
      if (mTarget)
         mTarget->SetValue(1000/TheScale->PitchToFreq(mPitch+mBend));
   }
}

void NoteToMs::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void NoteToMs::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToMs::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);
}
