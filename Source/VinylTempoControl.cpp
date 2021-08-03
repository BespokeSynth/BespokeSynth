/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
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
