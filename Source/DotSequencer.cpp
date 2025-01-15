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
//  DotSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/23.
//
//

#include "DotSequencer.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "FileStream.h"
#include "Scale.h"

DotSequencer::DotSequencer()
{
}

void DotSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheTransport->AddAudioPoller(this);
}

void DotSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mColsSlider, "cols", &mCols, 1, 64);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mRowsSlider, "rows", &mRows, 1, 64);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNoteModeSelector, "notemode", (int*)(&mNoteMode), 80);

   UIBLOCK_NEWLINE();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, 0, 8);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mRowOffsetSlider, "row offset", &mRowOffset, -12, 12);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mClearButton, "clear");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mDoubleButton, "double");

   UIBLOCK_NEWLINE();
   UIBLOCK_SHIFTX(40);
   UIBLOCK_SHIFTY(25);
   UICONTROL_CUSTOM(mDotGrid, new DotGrid(UICONTROL_BASICS("dotgrid"), 210, 210, mCols, mRows));
   ENDUIBLOCK(mWidth, mHeight);

   AddUIControl(mDotGrid);
   mDotGrid->SetMajorColSize(4);

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

   mNoteModeSelector->AddLabel("scale", (int)NoteMode::Scale);
   mNoteModeSelector->AddLabel("chromatic", (int)NoteMode::Chromatic);

   Resize(mWidth, mHeight);
}

DotSequencer::~DotSequencer()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void DotSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofSetLineWidth(mDotGrid->GetDotSize() * .25f);
   float ysize = mHeight / mDotGrid->GetRows();
   for (int i = 0; i < mDotGrid->GetRows(); ++i)
   {
      ofVec2f pos = mDotGrid->GetCellPosition(0, i - 1) + mDotGrid->GetPosition(true);
      ofVec2f lineAcrossStart(pos.x + 5, pos.y - ysize * .5f + 2);
      ofVec2f lineAcrossEnd(pos.x + mDotGrid->GetWidth() - 10, pos.y - ysize * .5f + 2);

      if (RowToPitch(i) % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot() % TheScale->GetPitchesPerOctave())
      {
         ofSetColor(0, 255, 0, gModuleDrawAlpha * .05f);
         ofLine(lineAcrossStart, lineAcrossEnd);
         ofSetColor(0, 255, 0, gModuleDrawAlpha * .8f);
      }
      else if (TheScale->GetPitchesPerOctave() == 12 && RowToPitch(i) % TheScale->GetPitchesPerOctave() == (TheScale->ScaleRoot() + 7) % TheScale->GetPitchesPerOctave())
      {
         ofSetColor(200, 150, 0, gModuleDrawAlpha * .05f);
         ofLine(lineAcrossStart, lineAcrossEnd);
         ofSetColor(200, 150, 0, gModuleDrawAlpha * .8f);
      }
      else if (mNoteMode == NoteMode::Chromatic && TheScale->IsInScale(RowToPitch(i)))
      {
         ofSetColor(175, 100, 0, gModuleDrawAlpha * .05f);
         ofLine(lineAcrossStart, lineAcrossEnd);
         ofSetColor(175, 100, 0, gModuleDrawAlpha * .8f);
      }
      else
      {
         ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
      }

      float scale = std::min(mDotGrid->IClickable::GetDimensions().y / mDotGrid->GetRows() - 2, 10.0f);
      DrawTextRightJustify(NoteName(RowToPitch(i), false, true) + "(" + ofToString(RowToPitch(i)) + ")", pos.x - 3, pos.y - ysize * .5f + (scale / 2), scale);
   }
   ofPopStyle();

   mIntervalSelector->Draw();
   mNoteModeSelector->Draw();
   mClearButton->Draw();
   mOctaveSlider->Draw();
   mColsSlider->Draw();
   mRowsSlider->Draw();
   mRowOffsetSlider->Draw();
   mDoubleButton->Draw();
   mDotGrid->Draw();
}

void DotSequencer::OnTimeEvent(double time)
{
   if (mHasExternalPulseSource)
      return;

   OnStep(time, 1, 0);
}

void DotSequencer::OnStep(double time, float velocity, int flags)
{
   if (mEnabled)
   {
      mStepIdx = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mDotGrid->GetCols());
      mDotGrid->SetHighlightCol(time, mStepIdx);

      for (int row = 0; row < mDotGrid->GetRows(); ++row)
      {
         const DotGrid::DotData& data = mDotGrid->GetDataAt(mStepIdx, row);
         if (data.mOn)
         {
            int pitch = RowToPitch(row);
            if (pitch >= 0 && pitch < 128)
            {
               bool played = false;
               for (int i = 0; i < (int)mPlayingDots.size(); ++i)
               {
                  if (!played && mPlayingDots[i].mPitch == -1)
                  {
                     mPlayingDots[i].mPitch = pitch;
                     mPlayingDots[i].mRow = row;
                     mPlayingDots[i].mCol = mStepIdx;
                     mPlayingDots[i].mPlayedTime = time;
                     played = true;
                  }
                  else if (mPlayingDots[i].mPitch == pitch)
                  {
                     mNoteOutput.PlayNote(NoteMessage(time, pitch, 0)); //note off any colliding pitches
                     mPlayingDots[i].mPitch = -1;
                  }
               }

               mNoteOutput.PlayNote(NoteMessage(time, pitch, std::max(int(data.mVelocity * 127), 1)));
               mDotGrid->OnPlayed(time, mStepIdx, row);
            }
         }
      }
   }
}

void DotSequencer::OnTransportAdvanced(float amount)
{
   for (int i = 0; i < (int)mPlayingDots.size(); ++i)
   {
      if (mPlayingDots[i].mPitch != -1)
      {
         const DotGrid::DotData& data = mDotGrid->GetDataAt(mPlayingDots[i].mCol, mPlayingDots[i].mRow);
         double noteOffTime = -1;

         if (data.mOn == false || mStepIdx < mPlayingDots[i].mCol)
         {
            noteOffTime = NextBufferTime(!K(includeLookahead));
         }
         else
         {
            double noteEnd = mPlayingDots[i].mPlayedTime + TheTransport->GetDuration(mInterval) * std::max(.5f, data.mLength);
            if (noteEnd < NextBufferTime(K(includeLookahead)))
               noteOffTime = noteEnd;
         }

         if (noteOffTime != -1)
         {
            mNoteOutput.PlayNote(NoteMessage(noteOffTime, mPlayingDots[i].mPitch, 0));
            mPlayingDots[i].mPitch = -1;
         }
      }
   }

   if (mShouldStopAllNotes)
   {
      for (int i = 0; i < (int)mPlayingDots.size(); ++i)
      {
         if (mPlayingDots[i].mPitch != -1)
         {
            mNoteOutput.PlayNote(NoteMessage(NextBufferTime(!K(includeLookahead)), mPlayingDots[i].mPitch, 0));
            mPlayingDots[i].mPitch = -1;
         }
      }
      mShouldStopAllNotes = false;
   }
}

int DotSequencer::RowToPitch(int row) const
{
   row += mRowOffset;

   int numTonesInScale = TheScale->NumTonesInScale();
   switch (mNoteMode)
   {
      case NoteMode::Scale:
         return TheScale->GetPitchFromTone(row + mOctave * numTonesInScale + TheScale->GetScaleDegree());
      case NoteMode::Chromatic:
         return row + mOctave * TheScale->GetPitchesPerOctave();
   }
   return row;
}

bool DotSequencer::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mDotGrid->NotifyMouseScrolled(x, y, scrollX, scrollY, isSmoothScroll, isInvertedScroll);
   return false;
}

void DotSequencer::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;

   ofVec2f gridPos = mDotGrid->GetPosition(K(local));

   mDotGrid->SetDimensions(w - 8 - gridPos.x, h - 5 - gridPos.y);
}

void DotSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void DotSequencer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
      mDotGrid->Clear();
   if (button == mDoubleButton)
   {
      if (mCols * 2 <= mDotGrid->GetMaxColumns())
      {
         mCols *= 2;
         mDotGrid->SetGrid(mCols, mRows);
         for (int col = 0; col < mCols / 2; ++col)
         {
            for (int row = 0; row < mRows; ++row)
               mDotGrid->CopyDot(DotGrid::DotPosition(col, row), DotGrid::DotPosition(col + mCols / 2, row));
         }
      }
   }
}

void DotSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
      if (Transport::IsTripletInterval(mInterval))
         mDotGrid->SetMajorColSize(3);
      else
         mDotGrid->SetMajorColSize(4);
   }
}

void DotSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mColsSlider || slider == mRowsSlider)
      mDotGrid->SetGrid(mCols, mRows);
}

void DotSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      if (!mEnabled)
         mShouldStopAllNotes = true;
   }
}

void DotSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void DotSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void DotSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mInterval;
   out << mHasExternalPulseSource;
   out << mWidth;
   out << mHeight;

   mDotGrid->SaveState(out);
}

void DotSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   int interval;
   in >> interval;
   mInterval = (NoteInterval)interval;

   in >> mHasExternalPulseSource;
   in >> mWidth;
   in >> mHeight;
   Resize(mWidth, mHeight);

   mDotGrid->LoadState(in);
}
