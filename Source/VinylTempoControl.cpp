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
: IAudioProcessor(gBufferSize)
, mReferencePitch(1)
, mVinylControl(gSampleRate)
, mUseVinylControl(false)
, mUseVinylControlCheckbox(nullptr)
, mSpeed(1)
{
   //mModulationBuffer = new float[gBufferSize];
}

VinylTempoControl::~VinylTempoControl()
{
   //delete[] mModulationBuffer;
}

void VinylTempoControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mUseVinylControlCheckbox = new Checkbox(this,"control",4,2,&mUseVinylControl);

   GetPatchCableSource()->SetEnabled(false);

   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

void VinylTempoControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mUseVinylControlCheckbox->Draw();
   
   if (CanStartVinylControl())
      DrawTextNormal(ofToString(mVinylControl.GetPitch(),2),60,14);
}

void VinylTempoControl::Process(double time)
{
   PROFILER(VinylTempoControl);
   
   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   assert(GetBuffer()->BufferSize());

   if (GetBuffer()->NumActiveChannels() >= 2)
   {
      mVinylControl.Process(GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(1), gBufferSize);

      if (mUseVinylControl)
      {
         float speed = mVinylControl.GetPitch() / mReferencePitch;
         if (speed == 0 || mVinylControl.GetStopped())
            speed = .0001f;
         mSpeed = speed;
      }
      else
      {
         mReferencePitch = mVinylControl.GetPitch();
      }
   }

   GetBuffer()->Reset();
}

void VinylTempoControl::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float VinylTempoControl::Value(int samplesIn)
{
   //return mModulationBuffer[samplesIn];
   return mSpeed;
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

void VinylTempoControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);

   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();

   moduleInfo["target"] = targetPath;
}

void VinylTempoControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VinylTempoControl::SetUpFromSaveData()
{
}
