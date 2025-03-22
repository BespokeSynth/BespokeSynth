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
//  VelocityStepSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/14/14.
//
//

#include "VelocityStepSequencer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "MidiController.h"

#define ARP_REST -100
#define ARP_HOLD -101

VelocityStepSequencer::VelocityStepSequencer()
{
}

void VelocityStepSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(-.1f, true), false);
}

void VelocityStepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this, "interval", 115, 2, (int*)(&mInterval));
   mLengthSlider = new IntSlider(this, "len", 115, 20, 40, 15, &mLength, 1, VSS_MAX_STEPS);
   mResetOnDownbeatCheckbox = new Checkbox(this, "downbeat", 5, 20, &mResetOnDownbeat);

   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("1n", kInterval_1n);

   for (int i = 0; i < VSS_MAX_STEPS; ++i)
   {
      mVels[i] = 80;
      mVelSliders[i] = new IntSlider(this, ("vel" + ofToString(i + 1)).c_str(), 10, 35 + i * 15, 80, 15, &(mVels[i]), 1, 127);
   }
}

VelocityStepSequencer::~VelocityStepSequencer()
{
   TheTransport->RemoveListener(this);
}

void VelocityStepSequencer::SetMidiController(std::string name)
{
   if (mController)
      mController->RemoveListener(this);
   mController = TheSynth->FindMidiController(name);
   if (mController)
      mController->AddListener(this, 0);
}

void VelocityStepSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mResetOnDownbeatCheckbox->Draw();

   for (int i = 0; i < VSS_MAX_STEPS; ++i)
      mVelSliders[i]->Draw();

   ofPushStyle();
   ofSetColor(0, 255, 0, 50);
   ofFill();
   ofRect(10, 35 + mArpIndex * 15, 80, 15);
   ofPopStyle();
}

void VelocityStepSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void VelocityStepSequencer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity > 0)
      note.velocity = mCurrentVelocity;
   PlayNoteOutput(note);
}

void VelocityStepSequencer::OnTimeEvent(double time)
{
   ++mArpIndex;

   if (mArpIndex >= mLength)
      mArpIndex = 0;

   if (mResetOnDownbeat && TheTransport->GetQuantized(time, mTransportListenerInfo) == 0)
      mArpIndex = 0;

   mCurrentVelocity = mVels[mArpIndex];
}

void VelocityStepSequencer::OnMidiNote(MidiNote& note)
{
}

void VelocityStepSequencer::OnMidiControl(MidiControl& control)
{
   if (!mEnabled)
      return;

   if (control.mControl >= 41 && control.mControl <= 48)
   {
      int step = control.mControl - 41;
      if (control.mValue >= 1)
         mVels[step] = control.mValue;
   }
}

void VelocityStepSequencer::ButtonClicked(ClickButton* button, double time)
{
}

void VelocityStepSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
      {
         transportListenerInfo->mInterval = mInterval;
         transportListenerInfo->mOffsetInfo = OffsetInfo(-.1f, true);
      }
   }
}

void VelocityStepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void VelocityStepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);

   SetUpFromSaveData();
}

void VelocityStepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetMidiController(mModuleSaveData.GetString("controller"));
}
