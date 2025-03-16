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
//  Polyrhythms.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/12/13.
//
//

#include "Polyrhythms.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"

Polyrhythms::Polyrhythms()
{
   mHeight = mRhythmLines.size() * 17 + 5;
}

void Polyrhythms::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void Polyrhythms::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i = 0; i < (int)mRhythmLines.size(); ++i)
   {
      mRhythmLines[i] = new RhythmLine(this, i);
      mRhythmLines[i]->CreateUIControls();
   }
}

Polyrhythms::~Polyrhythms()
{
   TheTransport->RemoveAudioPoller(this);

   for (int i = 0; i < mRhythmLines.size(); ++i)
      delete mRhythmLines[i];
}

void Polyrhythms::OnTransportAdvanced(float amount)
{
   PROFILER(Polyrhythms);

   if (!mEnabled)
      return;

   double time = NextBufferTime(true);

   for (int i = 0; i < mRhythmLines.size(); ++i)
   {
      int beats = mRhythmLines[i]->mGrid->GetCols();

      TransportListenerInfo info(nullptr, kInterval_CustomDivisor, OffsetInfo(0, false), false);
      info.mCustomDivisor = beats;

      double remainderMs;
      int oldStep = TheTransport->GetQuantized(NextBufferTime(true) - gBufferSizeMs, &info);
      int newStep = TheTransport->GetQuantized(NextBufferTime(true), &info, &remainderMs);
      float val = mRhythmLines[i]->mGrid->GetVal(newStep, 0);
      if (newStep != oldStep && val > 0)
         PlayNoteOutput(NoteMessage(time - remainderMs, mRhythmLines[i]->mPitch, val * 127));

      mRhythmLines[i]->mGrid->SetHighlightCol(time, newStep);
   }
}

void Polyrhythms::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->Draw();
}

void Polyrhythms::Resize(float w, float h)
{
   mWidth = MAX(150, w);
   mHeight = mRhythmLines.size() * 17 + 5;
   for (int i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->OnResize();
}

void Polyrhythms::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   for (int i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->OnClicked(x, y, right);
}

void Polyrhythms::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (int i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->MouseReleased();
}

bool Polyrhythms::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   for (int i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->MouseMoved(x, y);
   return false;
}

void Polyrhythms::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   for (int i = 0; i < mRhythmLines.size(); ++i)
   {
      if (list == mRhythmLines[i]->mLengthSelector)
         mRhythmLines[i]->UpdateGrid();
   }
}

void Polyrhythms::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Polyrhythms::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void Polyrhythms::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mRhythmLines.size();
   for (size_t i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->mGrid->SaveState(out);
}

void Polyrhythms::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int size;
   in >> size;
   LoadStateValidate(size == (int)mRhythmLines.size());
   for (size_t i = 0; i < mRhythmLines.size(); ++i)
      mRhythmLines[i]->mGrid->LoadState(in);
}


RhythmLine::RhythmLine(Polyrhythms* owner, int index)
: mIndex(index)
, mPitch(index)
, mOwner(owner)
{
}

void RhythmLine::CreateUIControls()
{
   mGrid = new UIGrid(mOwner, "uigrid", 4, 4 + mIndex * 17, 100, 15, 4, 1);
   mLengthSelector = new DropdownList(mOwner, ("length" + ofToString(mIndex)).c_str(), -1, -1, &mLength);
   mNoteSelector = new TextEntry(mOwner, ("note" + ofToString(mIndex)).c_str(), -1, -1, 4, &mPitch, 0, 127);

   mLengthSelector->AddLabel("3", 3);
   mLengthSelector->AddLabel("4", 4);
   mLengthSelector->AddLabel("5", 5);
   mLengthSelector->AddLabel("6", 6);
   mLengthSelector->AddLabel("7", 7);
   mLengthSelector->AddLabel("8", 8);
   mLengthSelector->AddLabel("9", 9);
   mLengthSelector->AddLabel("3x4", 12);
   mLengthSelector->AddLabel("4x4", 16);
   mLengthSelector->AddLabel("5x4", 20);
   mLengthSelector->AddLabel("6x4", 24);
   mLengthSelector->AddLabel("7x4", 28);
   mLengthSelector->AddLabel("8x4", 32);
   mLengthSelector->AddLabel("9x4", 36);

   mGrid->SetGridMode(UIGrid::kMultisliderBipolar);
   mGrid->SetRequireShiftForMultislider(true);

   OnResize();
}

void RhythmLine::OnResize()
{
   mGrid->SetDimensions(mOwner->IClickable::GetDimensions().x - 100, 15);
   mLengthSelector->PositionTo(mGrid, kAnchor_Right);
   mNoteSelector->PositionTo(mLengthSelector, kAnchor_Right);
}

void RhythmLine::UpdateGrid()
{
   mGrid->SetGrid(mLength, 1);
   if (mLength % 4 == 0)
      mGrid->SetMajorColSize(mLength / 4);
   else
      mGrid->SetMajorColSize(-1);
}

void RhythmLine::Draw()
{
   mGrid->Draw();
   mLengthSelector->Draw();
   mNoteSelector->Draw();
}

void RhythmLine::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   mGrid->TestClick(x, y, right);
}

void RhythmLine::MouseReleased()
{
   mGrid->MouseReleased();
}

void RhythmLine::MouseMoved(float x, float y)
{
   mGrid->NotifyMouseMoved(x, y);
}
