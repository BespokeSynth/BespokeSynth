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
//  TapTempo.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 10/25/24.
//
//

#include "TapTempo.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "ModulationChain.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

TapTempo::TapTempo()
{
}

TapTempo::~TapTempo()
{
}

void TapTempo::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   UICONTROL_CUSTOM(mWindowEntry, new TextEntry(UICONTROL_BASICS("window"), 7, &mWindow, 1, 99999); mWindowEntry->DrawLabel(false););
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mResetButton, new ClickButton(UICONTROL_BASICS("reset")));
   ENDUIBLOCK(mWidth, mHeight);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

void TapTempo::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mWindowEntry->Draw();
   mReset->Draw();
}

void TapTempo::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void TapTempo::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mLastBeatTime = time;
      if (!mCount)
         mFirstBeatTime = time;
      mCount++;
   }
}

void TapTempo::OnPulse(double time, float velocity, int flags)
{
   if (mEnabled && velocity > 0)
   {
      mLastBeatTime = time;
      if (!mCount)
         mFirstBeatTime = time;
      mCount++;
   }
}

void TapTempo::TextEntryComplete(TextEntry* entry)
{ // TODO: Resize the array here
}

float TapTempo::Value(int samplesIn)
{
   if (mCount < 2)
      return 0;

   return (mCount - 1) / ((mLastBeatTime - mFirstBeatTime) / 60 / 1000) * 4;
}

void TapTempo::ButtonClicked(ClickButton* button, double time)
{
   mCount = 0;
   mFirstBeatTime = 0;
   mLastBeatTime = 0;
}

void TapTempo::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void TapTempo::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void TapTempo::SetUpFromSaveData()
{
}
