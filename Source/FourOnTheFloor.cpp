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
//  FourOnTheFloor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 6/23/13.
//
//

#include "FourOnTheFloor.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"

FourOnTheFloor::FourOnTheFloor()
{
}

void FourOnTheFloor::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_4n, OffsetInfo(0, true), true);
}

void FourOnTheFloor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTwoOnTheFloorCheckbox = new Checkbox(this, "two", 4, 2, &mTwoOnTheFloor);
}

FourOnTheFloor::~FourOnTheFloor()
{
   TheTransport->RemoveListener(this);
}

void FourOnTheFloor::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   mTwoOnTheFloorCheckbox->Draw();
}

void FourOnTheFloor::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   int kick = 0;
   PlayNoteOutput(NoteMessage(time, kick, 127));
}

void FourOnTheFloor::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mTwoOnTheFloorCheckbox)
   {
      if (mTwoOnTheFloor)
      {
         TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
         if (transportListenerInfo != nullptr)
            transportListenerInfo->mInterval = kInterval_2n;
      }
      else
      {
         TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
         if (transportListenerInfo != nullptr)
            transportListenerInfo->mInterval = kInterval_4n;
      }
   }
}

void FourOnTheFloor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FourOnTheFloor::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
