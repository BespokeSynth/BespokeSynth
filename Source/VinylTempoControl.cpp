//
//  VinylTempoControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/18/14.
//
//

#include "VinylTempoControl.h"
#include "OpenFrameworksPort.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "Profiler.h"

VinylTempoControl* TheVinylTempoControl = nullptr;

VinylTempoControl::VinylTempoControl()
: mReferencePitch(1)
, mReferenceTempo(120)
, mVinylControl(gSampleRate)
, mVinylControlInLeft(nullptr)
, mVinylControlInRight(nullptr)
, mUseVinylControl(false)
, mUseVinylControlCheckbox(nullptr)
, mLeftChannel(0)
, mRightChannel(1)
{
   assert(TheVinylTempoControl == nullptr);
   TheVinylTempoControl = this;
   TheTransport->AddAudioPoller(this);
}

VinylTempoControl::~VinylTempoControl()
{
   assert(TheVinylTempoControl == this || TheVinylTempoControl == nullptr);
   TheVinylTempoControl = nullptr;
   TheTransport->RemoveAudioPoller(this);
}

void VinylTempoControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mUseVinylControlCheckbox = new Checkbox(this,"control",4,2,&mUseVinylControl);
}

void VinylTempoControl::DrawModule()
{
   DrawConnection(TheTransport);
   if (Minimized() || IsVisible() == false)
      return;
   
   mUseVinylControlCheckbox->Draw();
   
   if (CanStartVinylControl())
      DrawTextNormal(ofToString(mVinylControl.GetPitch(),2),60,14);
}

void VinylTempoControl::OnTransportAdvanced(float amount)
{
   PROFILER(VinylTempoControl);
   
   if (!mEnabled)
      return;
   
   if (mVinylControlInLeft && mVinylControlInRight)
   {
      mVinylControl.Process(mVinylControlInLeft, mVinylControlInRight, gBufferSize);
      
      if (mUseVinylControl)
      {
         float speed = mVinylControl.GetPitch() / mReferencePitch;
         if (speed == 0 || mVinylControl.GetStopped())
            speed = .0001f;
         TheTransport->SetTempo(speed * mReferenceTempo);
      }
      else
      {
         mReferencePitch = mVinylControl.GetPitch();
         mReferenceTempo = TheTransport->GetTempo();
      }
   }
}

void VinylTempoControl::SetVinylControlInput(float *left, float *right, int numSamples)
{
   assert(numSamples == gBufferSize);
   mVinylControlInLeft = left;
   mVinylControlInRight = right;
}

bool VinylTempoControl::CanStartVinylControl()
{
   return !mVinylControl.GetStopped() && fabsf(mVinylControl.GetPitch()) > .001f;
}

void VinylTempoControl::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mUseVinylControlCheckbox)
   {
      if (!CanStartVinylControl())
         mUseVinylControl = false;
   }
}

void VinylTempoControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("leftchannel", moduleInfo, 1, 1, MAX_INPUT_CHANNELS);
   mModuleSaveData.LoadInt("rightchannel", moduleInfo, 2, 1, MAX_INPUT_CHANNELS);
   
   SetUpFromSaveData();
}

void VinylTempoControl::SetUpFromSaveData()
{
   mLeftChannel = mModuleSaveData.GetInt("leftchannel");
   mRightChannel = mModuleSaveData.GetInt("rightchannel");
}
