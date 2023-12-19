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
//  PitchToValue.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#include "PitchToValue.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PitchToValue::PitchToValue()
: mControlCable(nullptr)
{
}

PitchToValue::~PitchToValue()
{
}

void PitchToValue::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControlCable = new PatchCableSource(this, kConnectionType_Modulator); // CHECKME: Need to set modulator owner?
   AddPatchCableSource(mControlCable);
}

void PitchToValue::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PitchToValue::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void PitchToValue::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mTarget) // CHECKME: Enabled? velocity > 0?
   {
      mTarget->SetValue(pitch);
      mControlCable->AddHistoryEvent(gTime, true);
      mControlCable->AddHistoryEvent(gTime + 15, false);
   }
}

void PitchToValue::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   std::string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void PitchToValue::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchToValue::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);
}
