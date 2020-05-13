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
#include "FillSaveDropdown.h"
#include "PolyphonyMgr.h"
#include "PatchCableSource.h"

NoteChainNode::NoteChainNode()
: mTriggerButton(nullptr)
, mPitch(48)
, mVelocity(1)
, mDuration(100)
, mPitchEntry(nullptr)
, mVelocitySlider(nullptr)
, mStartTime(0)
, mNoteOn(false)
, mWaitingToTrigger(false)
, mNextSelector(nullptr)
, mNextInterval(kInterval_8n)
, mQueueTrigger(false)
{
   SetIsNoteOrigin(true);
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
   mPitchEntry = new TextEntry(this,"pitch",5,3,3,&mPitch,0,127);
   mTriggerButton = new ClickButton(this,"trigger",50,3);
   mVelocitySlider = new FloatSlider(this,"velocity",5,21,100,15,&mVelocity,0,1);
   mDurationSlider = new FloatSlider(this,"duration",5,39,100,15,&mDuration,1,1000);
   mNextSelector = new DropdownList(this,"next",5,57,(int*)(&mNextInterval));
   
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
   
   mNextNodeCable = new PatchCableSource(this, kConnectionType_Special);
   mNextNodeCable->AddTypeFilter("notechain");
   mNextNodeCable->SetManualPosition(100, 10);
   mNextNodeCable->SetAllowMultipleTargets(true);
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
      TriggerNote();
      mQueueTrigger = false;
   }
}

void NoteChainNode::OnTransportAdvanced(float amount)
{
   if (mNoteOn && gTime > mStartTime + mDuration)
   {
      mNoteOn = false;
      mNoteOutput.Flush(gTime);
   }
   
   if (mWaitingToTrigger && gTime > mStartTime + mNext)
   {
      mWaitingToTrigger = false;
      for (auto* cable : mNextNodeCable->GetPatchCables())
      {
         NoteChainNode* node = dynamic_cast<NoteChainNode*>(cable->GetTarget());
         if (node)
            node->TriggerNote();
      }
   }
}

void NoteChainNode::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void NoteChainNode::TriggerNote()
{
   if (mEnabled)
   {
      mNoteOn = true;
      mWaitingToTrigger = true;
      mStartTime = gTime;
      mNext = TheTransport->GetDuration(mNextInterval);
      PlayNoteOutput(gTime, mPitch, mVelocity*127);
   }
}

void NoteChainNode::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteChainNode::ButtonClicked(ClickButton* button)
{
   if (button == mTriggerButton)
      mQueueTrigger = true;
}

void NoteChainNode::TextEntryComplete(TextEntry* entry)
{
   if (entry == mPitchEntry)
   {
      mNoteOn = false;
      mNoteOutput.Flush(gTime);
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
