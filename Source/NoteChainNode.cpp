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
//  NoteChainNode.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/16.
//
//

#include "NoteChainNode.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "PolyphonyMgr.h"
#include "PatchCableSource.h"

NoteChainNode::NoteChainNode()
{
}

void NoteChainNode::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
   TheTransport->AddListener(this, kInterval_8n, OffsetInfo(0, true), false);
}

NoteChainNode::~NoteChainNode()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteChainNode::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchEntry = new TextEntry(this, "pitch", 5, 3, 3, &mPitch, 0, 127);
   mTriggerButton = new ClickButton(this, "trigger", 50, 3);
   mVelocitySlider = new FloatSlider(this, "velocity", 5, 21, 100, 15, &mVelocity, 0, 1);
   mDurationSlider = new FloatSlider(this, "duration", 5, 39, 100, 15, &mDuration, 0.01f, 4, 4);
   mNextSelector = new DropdownList(this, "next", 5, 57, (int*)(&mNextInterval));

   mNextSelector->AddLabel("1n", kInterval_1n);
   mNextSelector->AddLabel("2n", kInterval_2n);
   mNextSelector->AddLabel("4n", kInterval_4n);
   mNextSelector->AddLabel("4nt", kInterval_4nt);
   mNextSelector->AddLabel("8n", kInterval_8n);
   mNextSelector->AddLabel("8nt", kInterval_8nt);
   mNextSelector->AddLabel("16n", kInterval_16n);
   mNextSelector->AddLabel("16nt", kInterval_16nt);
   mNextSelector->AddLabel("32n", kInterval_32n);
   mNextSelector->AddLabel("64n", kInterval_64n);

   mNextNodeCable = new PatchCableSource(this, kConnectionType_Pulse);
   mNextNodeCable->SetManualPosition(100, 10);
   AddPatchCableSource(mNextNodeCable);
}

void NoteChainNode::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchEntry->Draw();
   mTriggerButton->Draw();
   mVelocitySlider->Draw();
   mDurationSlider->Draw();
   mNextSelector->Draw();
}

void NoteChainNode::OnTimeEvent(double time)
{
   if (mQueueTrigger)
   {
      TriggerNote(time);
      mQueueTrigger = false;
   }
}

void NoteChainNode::OnTransportAdvanced(float amount)
{
   if (mNoteOn && NextBufferTime(true) > mStartTime + mDurationMs)
   {
      mNoteOn = false;
      mNoteOutput.Flush(mStartTime + mDurationMs);
   }

   if (mWaitingToTrigger && NextBufferTime(true) > mStartTime + mNext)
   {
      mWaitingToTrigger = false;
      DispatchPulse(mNextNodeCable, mStartTime + mNext, 1, 0);
   }
}

void NoteChainNode::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void NoteChainNode::OnPulse(double time, float velocity, int flags)
{
   TriggerNote(time);
}

void NoteChainNode::TriggerNote(double time)
{
   if (mEnabled)
   {
      mNoteOn = true;
      mWaitingToTrigger = true;
      mStartTime = time;
      mDurationMs = mDuration / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      mNext = TheTransport->GetDuration(mNextInterval);
      PlayNoteOutput(NoteMessage(time, mPitch, mVelocity * 127));
   }
}

void NoteChainNode::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteChainNode::ButtonClicked(ClickButton* button, double time)
{
   if (button == mTriggerButton)
      mQueueTrigger = true;
}

void NoteChainNode::TextEntryComplete(TextEntry* entry)
{
   if (entry == mPitchEntry)
   {
      mNoteOn = false;
      mNoteOutput.Flush(NextBufferTime(false));
   }
}

void NoteChainNode::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteChainNode::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
