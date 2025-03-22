/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2023 Ryan Challinor (contact: awwbees@gmail.com)

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
//  RhythmSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/5/23.
//
//

#include "RhythmSequencer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

RhythmSequencer::RhythmSequencer()
{
   mTransportPriority = ITimeListener::kTransportPriorityLate;
   for (size_t i = 0; i < mInputPitches.size(); ++i)
      mInputPitches[i] = false;
}

void RhythmSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(-.1f, true), false);
}

void RhythmSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLengthSlider, "length", &mLength, 1, kMaxSteps);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLinkLengthsCheckbox, "link lengths", &mLinkLengths);
   UIBLOCK_NEWLINE();

   UIBLOCK_PUSHSLIDERWIDTH(70);
   INTSLIDER(mLengthActionSlider, "len act", &mLengthAction, 1, kMaxSteps);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLengthVelSlider, "len vel", &mLengthVel, 1, kMaxSteps);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLengthOctaveSlider, "len oct", &mLengthOctave, 1, kMaxSteps);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLengthDegreeSlider, "len deg", &mLengthDegree, 1, kMaxSteps);
   UIBLOCK_NEWLINE();
   UIBLOCK_POPSLIDERWIDTH();

   UIBLOCK_PUSHSLIDERWIDTH(70);
   UIBLOCK_SHIFTY(5);
   for (int i = 0; i < kMaxSteps; ++i)
   {
      xOffset += 10;
      DROPDOWN(mStepData[i].mActionSelector, ("action " + ofToString(i + 1)).c_str(), (int*)(&(mStepData[i].mAction)), 60);
      UIBLOCK_SHIFTRIGHT();
      FLOATSLIDER_DIGITS(mStepData[i].mVelSlider, ("vel " + ofToString(i + 1)).c_str(), &(mStepData[i].mVel), 1.0f / 127.0f, 1.0f, 2);
      UIBLOCK_SHIFTRIGHT();
      INTSLIDER(mStepData[i].mOctaveSlider, ("octave " + ofToString(i + 1)).c_str(), &(mStepData[i].mOctave), -3, 3);
      UIBLOCK_SHIFTRIGHT();
      DROPDOWN(mStepData[i].mDegreeSelector, ("degree " + ofToString(i + 1)).c_str(), &(mStepData[i].mDegree), 70);
      UIBLOCK_NEWLINE();

      mStepData[i].mActionSelector->AddLabel("on", (int)StepAction::On);
      mStepData[i].mActionSelector->AddLabel("hold", (int)StepAction::Hold);
      mStepData[i].mActionSelector->AddLabel("off", (int)StepAction::Off);

      mStepData[i].mDegreeSelector->AddLabel("I", 0);
      mStepData[i].mDegreeSelector->AddLabel("II", 1);
      mStepData[i].mDegreeSelector->AddLabel("III", 2);
      mStepData[i].mDegreeSelector->AddLabel("IV", 3);
      mStepData[i].mDegreeSelector->AddLabel("V", 4);
      mStepData[i].mDegreeSelector->AddLabel("VI", 5);
      mStepData[i].mDegreeSelector->AddLabel("VII", 6);
   }
   UIBLOCK_POPSLIDERWIDTH();

   ENDUIBLOCK(mWidth, mHeight);

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
}

RhythmSequencer::~RhythmSequencer()
{
   TheTransport->RemoveListener(this);
}

void RhythmSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->SetShowing(!mHasExternalPulseSource);
   mIntervalSelector->Draw();
   mLengthSlider->SetShowing(mLinkLengths);
   mLengthSlider->Draw();
   mLinkLengthsCheckbox->Draw();
   mLengthActionSlider->SetShowing(!mLinkLengths);
   mLengthActionSlider->Draw();
   mLengthVelSlider->SetShowing(!mLinkLengths);
   mLengthVelSlider->Draw();
   mLengthOctaveSlider->SetShowing(!mLinkLengths);
   mLengthOctaveSlider->Draw();
   mLengthDegreeSlider->SetShowing(!mLinkLengths);
   mLengthDegreeSlider->Draw();

   for (int i = 0; i < kMaxSteps; ++i)
   {
      mStepData[i].mActionSelector->Draw();
      mStepData[i].mVelSlider->Draw();
      mStepData[i].mOctaveSlider->Draw();
      mStepData[i].mDegreeSelector->Draw();
   }

   ofPushStyle();
   if (mStepData[mArpIndexAction].mAction == StepAction::On)
      ofSetColor(0, 255, 0, 80);
   else if (DoesStepHold(mArpIndexAction, 0))
      ofSetColor(255, 255, 0, 80);
   else
      ofSetColor(255, 0, 0, 80);
   ofFill();
   if (mArpIndexAction >= 0 && mArpIndexAction < kMaxSteps)
      ofRect(mStepData[mArpIndexAction].mActionSelector->GetRect(true));
   if (mArpIndexVel >= 0 && mArpIndexVel < kMaxSteps)
      ofRect(mStepData[mArpIndexVel].mVelSlider->GetRect(true));
   if (mArpIndexOctave >= 0 && mArpIndexOctave < kMaxSteps)
      ofRect(mStepData[mArpIndexOctave].mOctaveSlider->GetRect(true));
   if (mArpIndexDegree >= 0 && mArpIndexDegree < kMaxSteps)
      ofRect(mStepData[mArpIndexDegree].mDegreeSelector->GetRect(true));

   ofSetColor(0, 255, 0, 255);
   for (int i = 0; i < (int)mStepData.size(); ++i)
   {
      if (mStepData[i].mAction == StepAction::On && i < mLengthAction)
      {
         ofRectangle rect = mStepData[i].mActionSelector->GetRect(true);
         ofCircle(rect.x - 6, rect.getCenter().y, 3);
      }

      if (DoesStepHold(i, 0) && i < mLengthAction)
      {
         ofRectangle rect = mStepData[i].mActionSelector->GetRect(true);
         ofLine(rect.x - 6, rect.getCenter().y + 1, rect.x - 6, rect.getCenter().y - 16);
      }
   }

   {
      ofSetColor(0, 0, 0, 100);
      ofRectangle rect = mStepData[(mLinkLengths ? mLength : mLengthAction) - 1].mActionSelector->GetRect(true);
      ofRect(rect.x, rect.getMaxY(), rect.width, mHeight - rect.getMaxY());
      rect = mStepData[(mLinkLengths ? mLength : mLengthVel) - 1].mVelSlider->GetRect(true);
      ofRect(rect.x, rect.getMaxY(), rect.width, mHeight - rect.getMaxY());
      rect = mStepData[(mLinkLengths ? mLength : mLengthOctave) - 1].mOctaveSlider->GetRect(true);
      ofRect(rect.x, rect.getMaxY(), rect.width, mHeight - rect.getMaxY());
      rect = mStepData[(mLinkLengths ? mLength : mLengthDegree) - 1].mDegreeSelector->GetRect(true);
      ofRect(rect.x, rect.getMaxY(), rect.width, mHeight - rect.getMaxY());
   }

   ofPopStyle();
}

bool RhythmSequencer::DoesStepHold(int index, int depth) const
{
   if (depth >= mLengthAction)
      return false;

   if (mStepData[index].mAction == StepAction::Hold)
   {
      int previous = (index + mLengthAction - 1) % mLengthAction;
      if (mStepData[previous].mAction == StepAction::On)
         return true;
      if (mStepData[previous].mAction == StepAction::Hold)
         return DoesStepHold(previous, depth + 1);
   }
   return false;
}

void RhythmSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
      for (int i = 0; i < 128; ++i)
         mInputPitches[i] = false;
   }
}

void RhythmSequencer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch >= 0 && note.pitch < 128)
      mInputPitches[note.pitch] = note.velocity > 0;
}

void RhythmSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;

   Step(time, velocity, flags);
}

void RhythmSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void RhythmSequencer::Step(double time, float velocity, int pulseFlags)
{
   if (!mEnabled)
      return;

   mArpIndex = GetArpIndex(time, mArpIndex, mLength, pulseFlags);
   if (mLinkLengths)
   {
      mArpIndexAction = mArpIndex;
      mArpIndexVel = mArpIndex;
      mArpIndexOctave = mArpIndex;
      mArpIndexDegree = mArpIndex;
      mLengthAction = mLength;
      mLengthVel = mLength;
      mLengthOctave = mLength;
      mLengthDegree = mLength;
   }
   else
   {
      mArpIndexAction = GetArpIndex(time, mArpIndexAction, mLengthAction, pulseFlags);
      mArpIndexVel = GetArpIndex(time, mArpIndexVel, mLengthVel, pulseFlags);
      mArpIndexOctave = GetArpIndex(time, mArpIndexOctave, mLengthOctave, pulseFlags);
      mArpIndexDegree = GetArpIndex(time, mArpIndexDegree, mLengthDegree, pulseFlags);
   }

   if (mEnabled)
   {
      bool* outputNotes = mNoteOutput.GetNotes();
      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (outputNotes[pitch] && mStepData[mArpIndexAction].mAction != StepAction::Hold)
            mNoteOutput.PlayNote(NoteMessage(time, pitch, 0));
      }

      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (mInputPitches[pitch] && mStepData[mArpIndexAction].mAction == StepAction::On)
         {
            int tone = TheScale->GetToneFromPitch(pitch) + mStepData[mArpIndexDegree].mDegree;
            int adjustedPitch = TheScale->GetPitchFromTone(tone) + mStepData[mArpIndexOctave].mOctave * 12;
            if (adjustedPitch >= 0 && adjustedPitch < 128)
               mNoteOutput.PlayNote(NoteMessage(time, adjustedPitch, mStepData[mArpIndexVel].mVel * velocity * 127.0f));
         }
      }
   }
}

int RhythmSequencer::GetArpIndex(double time, int current, int length, int pulseFlags)
{
   int direction = 1;
   if (pulseFlags & kPulseFlag_Backward)
      direction = -1;
   if (pulseFlags & kPulseFlag_Repeat)
      direction = 0;

   current += direction;
   if (direction > 0 && current >= length)
      current -= length;
   if (direction < 0 && current < 0)
      current += length;

   if (pulseFlags & kPulseFlag_Reset)
      current = 0;
   else if (pulseFlags & kPulseFlag_Random)
      current = gRandom() % length;

   if (!mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport))
   {
      current = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, length);
   }

   if (pulseFlags & kPulseFlag_Align)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int numMeasures = ceil(float(length) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      int step = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % length;
      current = step;
   }

   return current;
}

void RhythmSequencer::ButtonClicked(ClickButton* button, double time)
{
}

void RhythmSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
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

void RhythmSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLengthSlider)
      mLength = MIN(mLength, kMaxSteps);
}

void RhythmSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void RhythmSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void RhythmSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mHasExternalPulseSource;
}

void RhythmSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (rev >= 0)
      in >> mHasExternalPulseSource;
}
