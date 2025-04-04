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
/*
  ==============================================================================

    PlaySequencer.cpp
    Created: 12 Dec 2020 11:00:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PlaySequencer.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

PlaySequencer::PlaySequencer()
{
   mNoteOffScheduler.mOwner = this;
}

void PlaySequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
   TheTransport->AddListener(&mNoteOffScheduler, mInterval, OffsetInfo(TheTransport->GetMeasureFraction(mInterval) * .5f, false), false);
}

void PlaySequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mGridControlTarget = new GridControlTarget(this, "grid", mWidth - 50, 4);

   float width, height;
   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 50);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNumMeasuresSelector, "measures", &mNumMeasures, 50);
   UIBLOCK_NEWLINE();
   CHECKBOX(mWriteCheckbox, "write", &mWrite);
   CHECKBOX(mNoteRepeatCheckbox, "note repeat", &mNoteRepeat);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTUP();
   CHECKBOX(mLinkColumnsCheckbox, "link columns", &mLinkColumns);
   ENDUIBLOCK(width, height);

   UIBLOCK(3, height + 3, 45);
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      BUTTON(mSavedPatterns[i].mStoreButton, ("store" + ofToString(i)).c_str());
      BUTTON(mSavedPatterns[i].mLoadButton, ("load" + ofToString(i)).c_str());
      UIBLOCK_NEWCOLUMN();
   }
   ENDUIBLOCK(width, height);
   ofLog() << "width: " << width << " height: " << height;
   mGrid = new UIGrid(this, "uigrid", 3, height + 3, mWidth - 16, 150, TheTransport->CountInStandardMeasure(mInterval), (int)mLanes.size());
   mGrid->SetFlip(true);
   mGrid->SetGridMode(UIGrid::kMultisliderBipolar);
   mGrid->SetRequireShiftForMultislider(true);
   mGrid->SetRestrictDragToRow(true);

   ofRectangle gridRect = mGrid->GetRect(true);
   for (int i = 0; i < (int)mLanes.size(); ++i)
   {
      ofVec2f cellPos = mGrid->GetCellPosition(mGrid->GetCols() - 1, i) + mGrid->GetPosition(true);
      mLanes[i].mMuteOrEraseCheckbox = new Checkbox(this, ("mute/delete" + ofToString(i)).c_str(), gridRect.getMaxX() + 3, cellPos.y + 1, &mLanes[i].mMuteOrErase);
      mLanes[i].mMuteOrEraseCheckbox->SetDisplayText(false);
      mLanes[i].mMuteOrEraseCheckbox->SetBoxSize(10);
   }

   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   mNumMeasuresSelector->AddLabel("1", 1);
   mNumMeasuresSelector->AddLabel("2", 2);
   mNumMeasuresSelector->AddLabel("4", 4);
   mNumMeasuresSelector->AddLabel("8", 8);
   mNumMeasuresSelector->AddLabel("16", 16);
}

PlaySequencer::~PlaySequencer()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveListener(&mNoteOffScheduler);
}

void PlaySequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGridControlTarget->Draw();
   mIntervalSelector->Draw();
   mWriteCheckbox->Draw();
   mNoteRepeatCheckbox->Draw();
   mLinkColumnsCheckbox->Draw();
   mNumMeasuresSelector->Draw();
   for (size_t i = 0; i < mLanes.size(); ++i)
      mLanes[i].mMuteOrEraseCheckbox->Draw();
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      mSavedPatterns[i].mStoreButton->Draw();
      mSavedPatterns[i].mLoadButton->Draw();
      if (mSavedPatterns[i].mHasSequence)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(0, 255, 0, 80);
         ofRectangle rect = mSavedPatterns[i].mLoadButton->GetRect(K(local));
         ofRect(rect);
         ofPopStyle();
      }
   }

   mGrid->Draw();

   ofPushStyle();
   ofSetColor(255, 0, 0, 50);
   ofFill();
   for (int i = 0; i < (int)mLanes.size(); ++i)
   {
      if (mLanes[i].mMuteOrErase)
      {
         ofRectangle gridRect = mGrid->GetRect(true);
         ofVec2f cellPos = mGrid->GetCellPosition(0, i) + mGrid->GetPosition(true);
         ofRect(cellPos.x, cellPos.y + 1, gridRect.width, gridRect.height / mGrid->GetRows());
      }
   }
   ofPopStyle();
}

void PlaySequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   mGrid->TestClick(x, y, right);
}

void PlaySequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool PlaySequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   return false;
}

void PlaySequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (size_t i = 0; i < mLanes.size(); ++i)
   {
      if (checkbox == mLanes[i].mMuteOrEraseCheckbox)
      {
         if (mLinkColumns)
         {
            for (size_t j = 0; j < mLanes.size(); ++j)
            {
               if (j % 4 == i % 4)
                  mLanes[j].mMuteOrErase = mLanes[i].mMuteOrErase;
            }
         }
      }
   }
}

void PlaySequencer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (note.pitch < mLanes.size())
   {
      if (note.velocity > 0)
      {
         mLanes[note.pitch].mInputVelocity = note.velocity;
      }
      else
      {
         if (mNoteRepeat)
            mLanes[note.pitch].mInputVelocity = 0;
      }
   }
}

void PlaySequencer::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   int step = GetStep(time);

   mGrid->SetHighlightCol(time, step);

   for (int i = 0; i < (int)mLanes.size(); ++i)
   {
      float gridVal = mGrid->GetVal(step, i);
      int playVelocity = (int)(gridVal * 127);

      if (mLanes[i].mMuteOrErase)
      {
         playVelocity = 0;
         if (mWrite)
            mGrid->SetVal(step, i, 0);
      }

      if (mLanes[i].mInputVelocity > 0)
      {
         float velMult;
         switch (GetVelocityLevel())
         {
            case 1: velMult = mVelocityLight; break;
            case 2: velMult = mVelocityMed; break;
            default:
            case 3: velMult = mVelocityFull; break;
         }
         playVelocity = mLanes[i].mInputVelocity * velMult;
         if (mWrite)
            mGrid->SetVal(step, i, playVelocity / 127.0f);
         if (!mNoteRepeat)
            mLanes[i].mInputVelocity = 0;
      }

      if (playVelocity > 0 && mLanes[i].mIsPlaying == false)
      {
         PlayNoteOutput(NoteMessage(time, i, playVelocity));
         mLanes[i].mIsPlaying = true;
      }

      if (mSustain)
      {
         if (mLanes[i].mIsPlaying && playVelocity == 0)
         {
            PlayNoteOutput(NoteMessage(time, i, 0));
            mLanes[i].mIsPlaying = false;
         }
      }
   }

   UpdateLights();
}

void PlaySequencer::NoteOffScheduler::OnTimeEvent(double time)
{
   if (mOwner->mSustain)
      return;

   for (int i = 0; i < (int)mOwner->mLanes.size(); ++i)
   {
      if (mOwner->mLanes[i].mIsPlaying)
      {
         mOwner->PlayNoteOutput(NoteMessage(time, i, 0));
         mOwner->mLanes[i].mIsPlaying = false;
      }
   }

   mOwner->UpdateLights(true);
}

int PlaySequencer::GetStep(double time)
{
   int step = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mGrid->GetCols());
   return step;
}

void PlaySequencer::UpdateInterval()
{
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
   {
      transportListenerInfo->mInterval = mInterval;
      transportListenerInfo->mOffsetInfo = OffsetInfo(0, false);
   }

   TransportListenerInfo* noteOffListenerInfo = TheTransport->GetListenerInfo(&mNoteOffScheduler);
   if (noteOffListenerInfo != nullptr)
   {
      noteOffListenerInfo->mInterval = mInterval;
      noteOffListenerInfo->mOffsetInfo = OffsetInfo(TheTransport->GetMeasureFraction(mInterval) * .5f, false);
   }

   UpdateNumMeasures(mNumMeasures);
}

void PlaySequencer::UpdateNumMeasures(int oldNumMeasures)
{
   int oldSteps = mGrid->GetCols();

   int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
   mGrid->SetGrid(stepsPerMeasure * mNumMeasures, (int)mLanes.size());

   if (mNumMeasures > oldNumMeasures)
   {
      for (int i = 1; i < mNumMeasures / oldNumMeasures; ++i)
      {
         for (int j = 0; j < oldSteps; ++j)
         {
            for (int row = 0; row < mGrid->GetRows(); ++row)
            {
               mGrid->SetVal(j + i * oldSteps, row, mGrid->GetVal(j, row));
            }
         }
      }
   }
}

void PlaySequencer::UpdateLights(bool betweener)
{
   if (mGridControlTarget->GetGridController() == nullptr)
      return;

   IGridController* gridController = mGridControlTarget->GetGridController();

   for (int i = 0; i < 4; ++i)
      gridController->SetLight(i, 0, mWrite ? kGridColor2Bright : kGridColorOff);

   for (int i = 0; i < 3; ++i)
      gridController->SetLight(i, 1, mNoteRepeat && !betweener ? kGridColor2Bright : kGridColorOff);

   gridController->SetLight(3, 1, mLinkColumns ? kGridColor2Bright : kGridColorOff);

   gridController->SetLight(0, 2, mNumMeasures == 1 ? kGridColor3Bright : kGridColorOff);
   gridController->SetLight(1, 2, mNumMeasures == 2 ? kGridColor3Bright : kGridColorOff);
   gridController->SetLight(2, 2, mNumMeasures == 4 ? kGridColor3Bright : kGridColorOff);
   gridController->SetLight(3, 2, mNumMeasures == 8 ? kGridColor3Bright : kGridColorOff);

   gridController->SetLight(0, 3, GetVelocityLevel() == 1 || GetVelocityLevel() == 3 ? kGridColor2Bright : kGridColorOff);
   gridController->SetLight(1, 3, GetVelocityLevel() == 2 || GetVelocityLevel() == 3 ? kGridColor2Bright : kGridColorOff);

   gridController->SetLight(3, 3, mClearLane ? kGridColor1Bright : kGridColorOff);

   int step = GetStep(gTime);
   for (int i = 0; i < 16; ++i)
   {
      int x = (i / 4) % 4 + 4;
      int y = i % 4;
      gridController->SetLight(x, y, step % 16 == i ? kGridColor3Bright : kGridColorOff);
   }

   for (int i = 0; i < (int)mLanes.size(); ++i)
   {
      int x = i % 4 * 2;
      int y = 7 - i / 4;
      gridController->SetLight(x, y, mLanes[i].mIsPlaying ? kGridColor2Bright : kGridColorOff);
      gridController->SetLight(x + 1, y, mLanes[i].mMuteOrErase ? kGridColorOff : kGridColor1Bright);
   }
}

int PlaySequencer::GetVelocityLevel()
{
   if (mUseLightVelocity && !mUseMedVelocity)
      return 1;

   if (mUseMedVelocity && !mUseLightVelocity)
      return 2;

   return 3;
}

void PlaySequencer::OnControllerPageSelected()
{
   UpdateLights();
}

void PlaySequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (grid == mGridControlTarget->GetGridController())
   {
      bool press = velocity > 0;
      if (x >= 0 && y >= 0)
      {
         if (press && y == 0 && x < 4)
            mWrite = !mWrite;

         if (press && y == 1 && x < 3)
            mNoteRepeat = !mNoteRepeat;

         if (press && y == 1 && x == 3)
            mLinkColumns = !mLinkColumns;

         if (press && y == 2)
         {
            int newNumMeasures = mNumMeasures;
            if (x == 0)
               newNumMeasures = 1;
            if (x == 1)
               newNumMeasures = 2;
            if (x == 2)
               newNumMeasures = 4;
            if (x == 3)
               newNumMeasures = 8;

            if (newNumMeasures != mNumMeasures)
            {
               int oldNumMeasures = mNumMeasures;
               mNumMeasures = newNumMeasures;
               UpdateNumMeasures(oldNumMeasures);
            }
         }

         if (x == 0 && y == 3)
            mUseLightVelocity = press;
         if (x == 1 && y == 3)
            mUseMedVelocity = press;

         if (x == 3 && y == 3)
            mClearLane = press;

         if (x == 2 && y == 3 && mClearLane)
            mGrid->Clear();

         if (x >= 4 && y == 0)
            ButtonClicked(mSavedPatterns[x - 4].mStoreButton, NextBufferTime(false));
         if (x >= 4 && y == 1)
            ButtonClicked(mSavedPatterns[x - 4].mLoadButton, NextBufferTime(false));

         if (y >= 4)
         {
            int pitch = x / 2 + (7 - y) * 4;
            if (x % 2 == 0)
            {
               if (velocity > 0)
               {
                  mLanes[pitch].mInputVelocity = velocity * 127;
               }
               else
               {
                  if (mNoteRepeat)
                     mLanes[pitch].mInputVelocity = 0;
               }
            }
            else if (x % 2 == 1)
            {
               mLanes[pitch].mMuteOrErase = press;
               if (mLinkColumns)
               {
                  for (size_t i = 0; i < mLanes.size(); ++i)
                  {
                     if (i % 4 == pitch % 4)
                        mLanes[i].mMuteOrErase = press;
                  }
               }

               if (press && mClearLane)
               {
                  for (int i = 0; i < mGrid->GetCols(); ++i)
                  {
                     mGrid->SetVal(i, pitch, 0);

                     if (mLinkColumns)
                     {
                        for (int j = 0; j < (int)mLanes.size(); ++j)
                        {
                           if (j % 4 == pitch % 4)
                              mGrid->SetVal(i, j, 0);
                        }
                     }
                  }
               }
            }
         }

         UpdateLights();
      }
   }
}

void PlaySequencer::ButtonClicked(ClickButton* button, double time)
{
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      if (button == mSavedPatterns[i].mStoreButton)
      {
         mSavedPatterns[i].mNumMeasures = mNumMeasures;
         mSavedPatterns[i].mData = mGrid->GetData();
         mSavedPatterns[i].mHasSequence = false;
         for (auto& cell : mSavedPatterns[i].mData)
         {
            if (cell != 0)
            {
               mSavedPatterns[i].mHasSequence = true;
               break;
            }
         }
      }

      if (button == mSavedPatterns[i].mLoadButton)
      {
         mNumMeasures = mSavedPatterns[i].mNumMeasures;
         UpdateNumMeasures(mNumMeasures);
         mGrid->SetData(mSavedPatterns[i].mData);
      }
   }
}

void PlaySequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateInterval();
   if (list == mNumMeasuresSelector)
      UpdateNumMeasures(oldVal);
}

void PlaySequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

namespace
{
   const float extraW = 25;
   const float extraH = 100;
}

void PlaySequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void PlaySequencer::Resize(float w, float h)
{
   w = MAX(w - extraW, 219);
   h = MAX(h - extraH, 111);
   SetGridSize(w, h);

   ofRectangle gridRect = mGrid->GetRect(true);
   for (int i = 0; i < (int)mLanes.size(); ++i)
   {
      ofVec2f cellPos = mGrid->GetCellPosition(mGrid->GetCols() - 1, i) + mGrid->GetPosition(true);
      mLanes[i].mMuteOrEraseCheckbox->SetPosition(gridRect.getMaxX() + 3, cellPos.y + 1);
      mLanes[i].mMuteOrEraseCheckbox->SetBoxSize(MAX(10, mGrid->GetHeight() / mLanes.size()));
   }
}

void PlaySequencer::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
}

void PlaySequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("sustain", moduleInfo, false);
   mModuleSaveData.LoadFloat("velocity_full", moduleInfo, 1, 0, 1, K(isTextField));
   mModuleSaveData.LoadFloat("velocity_med", moduleInfo, .5f, 0, 1, K(isTextField));
   mModuleSaveData.LoadFloat("velocity_light", moduleInfo, .25f, 0, 1, K(isTextField));

   SetUpFromSaveData();
}

void PlaySequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mSustain = mModuleSaveData.GetBool("sustain");
   mVelocityFull = mModuleSaveData.GetFloat("velocity_full");
   mVelocityMed = mModuleSaveData.GetFloat("velocity_med");
   mVelocityLight = mModuleSaveData.GetFloat("velocity_light");
}

void PlaySequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mGrid->SaveState(out);

   out << (int)mSavedPatterns.size();
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      out << mSavedPatterns[i].mNumMeasures;
      out << mSavedPatterns[i].mHasSequence;
      out << (int)mSavedPatterns[i].mData.size();
      for (auto& data : mSavedPatterns[i].mData)
         out << data;
   }
}

void PlaySequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mGrid->LoadState(in);

   int numPatterns;
   in >> numPatterns;
   LoadStateValidate(numPatterns == mSavedPatterns.size());
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      in >> mSavedPatterns[i].mNumMeasures;
      in >> mSavedPatterns[i].mHasSequence;
      int size;
      in >> size;
      LoadStateValidate(size == (int)mSavedPatterns[i].mData.size());
      for (int j = 0; j < size; ++j)
         in >> mSavedPatterns[i].mData[j];
   }
}
