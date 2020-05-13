//
//  FourOnTheFloor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 6/23/13.
//
//

#include "FourOnTheFloor.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"

FourOnTheFloor::FourOnTheFloor()
: mTwoOnTheFloor(false)
, mTwoOnTheFloorCheckbox(nullptr)
{
   TheTransport->AddListener(this, kInterval_4n, OffsetInfo(0, true), true);
}

void FourOnTheFloor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTwoOnTheFloorCheckbox = new Checkbox(this,"two",4,2,&mTwoOnTheFloor);
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
   PlayNoteOutput(time, kick, 127, -1);
}

void FourOnTheFloor::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mTwoOnTheFloorCheckbox)
   {
      if (mTwoOnTheFloor)
         TheTransport->UpdateListener(this, kInterval_2n);
      else
         TheTransport->UpdateListener(this, kInterval_4n);
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

