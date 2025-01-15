//
//  M185Sequencer.cpp
//  Bespoke
//
//  Created by Lionel Landwerlin on 7/22/21.
//
//

#include "M185Sequencer.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "FileStream.h"

M185Sequencer::M185Sequencer()
{
}

void M185Sequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void M185Sequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK2(10, 0);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mResetStepButton, "reset step");
   int i = 0;
   for (auto& step : mSteps)
   {
      UIBLOCK_NEWLINE();
      step.xPos = 0;
      step.yPos = yPos;

      INTSLIDER(step.mPitchSlider, ("pitch" + ofToString(i)).c_str(), &step.mPitch, 0, 127);
      UIBLOCK_SHIFTRIGHT();
      INTSLIDER(step.mPulseCountSlider, ("pulses" + ofToString(i)).c_str(), &step.mPulseCount, 0, 8);
      UIBLOCK_SHIFTRIGHT();
      DROPDOWN(step.mGateSelector, ("gate" + ofToString(i)).c_str(), (int*)(&step.mGate), 60);

      step.mGateSelector->AddLabel("repeat", GateType::kGate_Repeat);
      step.mGateSelector->AddLabel("once", GateType::kGate_Once);
      step.mGateSelector->AddLabel("hold", GateType::kGate_Hold);
      step.mGateSelector->AddLabel("rest", GateType::kGate_Rest);

      ++i;
   }
   mWidth = UIBLOCKWIDTH();
   mHeight = UIBLOCKHEIGHT();
   ENDUIBLOCK0();

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("none", kInterval_None);
}

M185Sequencer::~M185Sequencer()
{
   TheTransport->RemoveListener(this);
}

void M185Sequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int totalSteps = 0;
   for (auto& step : mSteps)
      totalSteps += step.mPulseCount;
   DrawTextNormal("total steps: " + ofToString(totalSteps), 120, 13);

   ofPushStyle();
   for (int i = 0; i < mSteps.size(); i++)
   {
      ofFill();
      ofSetColor(0, i == mLastPlayedStepIdx ? 255 : 0, 0, gModuleDrawAlpha * .4f);
      ofRect(mSteps[i].xPos,
             mSteps[i].yPos,
             10, 10);
   }
   ofPopStyle();

   mResetStepButton->Draw();
   mIntervalSelector->Draw();
   for (auto& step : mSteps)
   {
      step.mPitchSlider->Draw();
      step.mPulseCountSlider->Draw();
      step.mGateSelector->Draw();
   }
}

void M185Sequencer::OnTimeEvent(double time)
{
   if (mHasExternalPulseSource)
      return;

   StepBy(time, 1, 0);
}

void M185Sequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   StepBy(time, velocity, flags);
}

void M185Sequencer::StepBy(double time, float velocity, int flags)
{
   if (flags & kPulseFlag_Reset)
      ResetStep();

   if (flags & kPulseFlag_SyncToTransport)
   {
      int totalSteps = 0;
      for (auto& step : mSteps)
         totalSteps += step.mPulseCount;
      int desiredStep = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, totalSteps);

      int stepsRemaining = desiredStep;
      mStepIdx = 0;
      for (auto& step : mSteps)
      {
         if (stepsRemaining < step.mPulseCount)
         {
            mStepPulseIdx = stepsRemaining;
            break;
         }
         stepsRemaining -= step.mPulseCount;
         ++mStepIdx;
      }
      if (mStepIdx >= mSteps.size())
      {
         mStepIdx = 0;
         mStepPulseIdx = 0;
      }
   }

   if (mEnabled)
   {
      bool stopPrevNote =
      mStepPulseIdx == 0 ||
      mSteps[mStepIdx].mGate == GateType::kGate_Repeat ||
      (mStepPulseIdx > 0 && mSteps[mStepIdx].mGate == GateType::kGate_Once);
      bool playNextNote =
      (mStepPulseIdx == 0 &&
       (mSteps[mStepIdx].mGate == GateType::kGate_Once ||
        mSteps[mStepIdx].mGate == GateType::kGate_Hold)) ||
      mSteps[mStepIdx].mGate == GateType::kGate_Repeat;

      if (mSteps[mStepIdx].mPulseCount == 0)
         playNextNote = false;

      if (stopPrevNote && mLastPitch >= 0)
      {
         PlayNoteOutput(NoteMessage(time, mLastPitch, 0));
         mLastPitch = -1;
      }
      if (playNextNote)
      {
         PlayNoteOutput(NoteMessage(time, mSteps[mStepIdx].mPitch, velocity * 127));
         mLastPitch = mSteps[mStepIdx].mPitch;
      }
   }
   else if (mLastPitch >= 0)
   {
      PlayNoteOutput(NoteMessage(time, mLastPitch, 0));
      mLastPitch = -1;
   }

   mLastPlayedStepIdx = mStepIdx;

   // Update step/pulse
   FindNextStep();
}

void M185Sequencer::FindNextStep()
{
   mStepPulseIdx++;
   int loopProtection = (int)mSteps.size() - 1;
   while (mStepPulseIdx >= mSteps[mStepIdx].mPulseCount)
   {
      mStepPulseIdx = 0;
      mStepIdx = (mStepIdx + 1) % mSteps.size();
      --loopProtection;
      if (loopProtection < 0)
         break;
   }
}

void M185Sequencer::ResetStep()
{
   mStepIdx = 0;
   mStepPulseIdx = 0;

   if (mSteps[mStepIdx].mPulseCount == 0) //if we don't have any pulses on the first step, find a step that does
      FindNextStep();
}

void M185Sequencer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void M185Sequencer::ButtonClicked(ClickButton* button, double time)
{
   if (mResetStepButton == button)
      ResetStep();
}

void M185Sequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void M185Sequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void M185Sequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void M185Sequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void M185Sequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (auto& step : mSteps)
   {
      out << step.mPitch;
      out << step.mPulseCount;
      out << (int)step.mGate;
   }
   out << (int)mInterval;
   out << mHasExternalPulseSource;
}

void M185Sequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   for (auto& step : mSteps)
   {
      in >> step.mPitch;
      in >> step.mPulseCount;

      int gate;
      in >> gate;
      step.mGate = (GateType)gate;
   }
   int interval;
   in >> interval;
   mInterval = (NoteInterval)interval;

   in >> mHasExternalPulseSource;
}
