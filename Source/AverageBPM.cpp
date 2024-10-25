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
//  AverageBPM.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 10/25/24.
//
//

#include "AverageBPM.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "ModulationChain.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

AverageBPM::AverageBPM()
{
}

AverageBPM::~AverageBPM()
{
}

void AverageBPM::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   BUTTON(mReset, "reset");
   ENDUIBLOCK(mWidth, mHeight);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

void AverageBPM::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void AverageBPM::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void AverageBPM::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mLastBeatTime = time;
      if (!mCount)
		 mFirstBeatTime = time;
	  mCount++;
   }
}

void AverageBPM::OnPulse(double time, float velocity, int flags)
{
   if (mEnabled && velocity > 0)
   {
      mLastBeatTime = time;
      if (!mCount)
		 mFirstBeatTime = time;
	  mCount++;
   }
}

float AverageBPM::Value(int samplesIn)
{
   if (mCount < 2)
	  return 0;

   return (mLastBeatTime - mFirstBeatTime) / (mCount - 1);
}

void AverageBPM::ButtonClicked(ClickButton* button, double time)
{
   mCount = 0;
   mFirstBeatTime = 0;
   mLastBeatTime = 0;
}

void AverageBPM::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AverageBPM::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void AverageBPM::SetUpFromSaveData()
{
}
