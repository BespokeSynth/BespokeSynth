//
//  NoteToFreq.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/17/15.
//
//

#include "NoteToFreq.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToFreq::NoteToFreq()
: mControlCable(nullptr)
, mPitch(0)
, mBend(0)
, mPitchBend(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

NoteToFreq::~NoteToFreq()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteToFreq::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mControlCable);
}

void NoteToFreq::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToFreq::PostRepatch(PatchCableSource* cableSource)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void NoteToFreq::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = pitchBend;
      if (mTarget)
         mTarget->SetValue(TheScale->PitchToFreq(mPitch+mBend));
   }
}

void NoteToFreq::OnTransportAdvanced(float amount)
{
   float bend = mPitchBend ? mPitchBend->GetValue(0) : 0;
   if (mEnabled && bend != mBend)
   {
      mBend = bend;
      if (mTarget)
         mTarget->SetValue(TheScale->PitchToFreq(mPitch+mBend));
   }
}

void NoteToFreq::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void NoteToFreq::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToFreq::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);
}
