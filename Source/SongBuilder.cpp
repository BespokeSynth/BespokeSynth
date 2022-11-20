/**
bespoke synth, a software modular synthesizer
Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
// Created by Ryan Challinor on 11/05/22.
//

#include "SongBuilder.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "FileStream.h"
#include "PatchCableSource.h"
#include "ofxJSONElement.h"

namespace
{
   const float kLeftMarginX = 3;
   const float kSongSequencerWidth = 175;
   const float kGridStartY = 20;
   const float kSectionTabWidth = 160;
   const float kTargetTabHeightTop = 30;
   const float kTargetTabHeightBottom = 10;
   const float kRowHeight = 20;
   const float kColumnWidth = 50;
   const float kSpacingX = 3;
   const float kSpacingY = 3;
}

SongBuilder::SongBuilder()
{
   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      mSequencerSectionId[i] = -1;
      mSequencerSectionSelector[i] = nullptr;
      mSequencerStepLength[i] = 4;
      mSequencerStepLengthEntry[i] = nullptr;
      mSequencerContextMenu[i] = nullptr;
      mSequencerContextMenuSelection[i] = ContextMenuItems::kNone;
   }
}

void SongBuilder::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(gBufferSizeMs, true), true);

   SetActiveSection(gTime, 0);
}

void SongBuilder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float width, height;
   UIBLOCK0();
   CHECKBOX(mUseSequencerCheckbox, "use sequencer", &mUseSequencer);
   BUTTON_STYLE(mPlaySequenceButton, "play", ButtonDisplayStyle::kPlay);
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mStopSequenceButton, "stop", ButtonDisplayStyle::kStop);
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mPauseSequenceButton, "pause", ButtonDisplayStyle::kPause);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLoopCheckbox, "loop", &mLoopSequence);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY_NUM(mSequenceLoopStartEntry, "loop start", 3, &mSequenceLoopStartIndex, 0, kMaxSequencerSections - 1);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY_NUM(mSequenceLoopEndEntry, "loop end", 3, &mSequenceLoopEndIndex, 0, kMaxSequencerSections - 1);
   ENDUIBLOCK(width, height);

   UIBLOCK(10, height + 4);
   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      DROPDOWN(mSequencerSectionSelector[i], ("section" + ofToString(i)).c_str(), &mSequencerSectionId[i], 80);
      mSequencerSectionSelector[i]->SetDrawTriangle(false);
      UIBLOCK_SHIFTRIGHT();
      TEXTENTRY_NUM(mSequencerStepLengthEntry[i], ("bars" + ofToString(i)).c_str(), 3, &mSequencerStepLength[i], 1, 999);
      UIBLOCK_SHIFTRIGHT();
      BUTTON_STYLE(mSequencerPlayFromButton[i], ("play from " + ofToString(i)).c_str(), ButtonDisplayStyle::kPlay);
      UIBLOCK_SHIFTRIGHT();
      DROPDOWN(mSequencerContextMenu[i], ("contextmenu" + ofToString(i)).c_str(), ((int*)&mSequencerContextMenuSelection[i]), 20);
      mSequencerContextMenu[i]->AddLabel("duplicate", (int)ContextMenuItems::kDuplicate);
      mSequencerContextMenu[i]->AddLabel("delete", (int)ContextMenuItems::kDelete);
      mSequencerContextMenu[i]->AddLabel("move up", (int)ContextMenuItems::kMoveUp);
      mSequencerContextMenu[i]->AddLabel("move down", (int)ContextMenuItems::kMoveDown);
      mSequencerContextMenu[i]->SetDisplayStyle(DropdownDisplayStyle::kHamburger);
      UIBLOCK_NEWLINE();
   }
   ENDUIBLOCK0();

   mChangeQuantizeSelector = new DropdownList(this, "change quantize", -1, -1, (int*)(&mChangeQuantizeInterval));
   mAddTargetButton = new ClickButton(this, "add target", -1, -1, ButtonDisplayStyle::kPlus);

   mChangeQuantizeSelector->AddLabel("none", kInterval_None);
   mChangeQuantizeSelector->AddLabel("1n", kInterval_1n);
   mChangeQuantizeSelector->AddLabel("2", kInterval_2);
   mChangeQuantizeSelector->AddLabel("3", kInterval_3);
   mChangeQuantizeSelector->AddLabel("4", kInterval_4);
}

SongBuilder::~SongBuilder()
{
   TheTransport->RemoveListener(this);
}

void SongBuilder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int gridStartX = kLeftMarginX;
   if (ShowSongSequencer())
      gridStartX += kSongSequencerWidth;

   mUseSequencerCheckbox->Draw();
   mChangeQuantizeSelector->SetPosition(gridStartX, kGridStartY + kTargetTabHeightTop - 15);
   mChangeQuantizeSelector->Draw();
   mAddTargetButton->SetPosition(gridStartX + kSectionTabWidth - 22, kGridStartY + 8);
   mAddTargetButton->Draw();
   mPlaySequenceButton->SetShowing(ShowSongSequencer());
   mPlaySequenceButton->Draw();
   mStopSequenceButton->SetShowing(ShowSongSequencer());
   mStopSequenceButton->Draw();
   mPauseSequenceButton->SetShowing(ShowSongSequencer());
   mPauseSequenceButton->Draw();
   mLoopCheckbox->SetShowing(ShowSongSequencer());
   mLoopCheckbox->Draw();
   mSequenceLoopStartEntry->SetShowing(ShowSongSequencer() && mLoopSequence);
   mSequenceLoopStartEntry->Draw();
   mSequenceLoopEndEntry->SetShowing(ShowSongSequencer() && mLoopSequence);
   mSequenceLoopEndEntry->Draw();

   for (int i = 0; i < (int)mSections.size(); ++i)
      mSections[i]->Draw(this, gridStartX, kGridStartY + kTargetTabHeightTop + kSpacingY + i * (kRowHeight + kSpacingY), i);

   for (int i = 0; i < (int)mTargets.size(); ++i)
      mTargets[i]->Draw(gridStartX + kSectionTabWidth + i * (kColumnWidth + kSpacingX), kGridStartY, (int)mSections.size());

   bool sequenceComplete = false;
   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      bool show = mUseSequencer && !sequenceComplete;
      mSequencerSectionSelector[i]->SetShowing(show);
      mSequencerStepLengthEntry[i]->SetShowing(show && mSequencerSectionId[i] != kSequenceEndId);
      mSequencerContextMenu[i]->SetShowing(show && mSequencerSectionId[i] >= 0);
      mSequencerPlayFromButton[i]->SetShowing(show && mSequencerSectionId[i] >= 0);
      mSequencerSectionSelector[i]->Draw();
      mSequencerStepLengthEntry[i]->Draw();
      mSequencerContextMenu[i]->Draw();
      mSequencerPlayFromButton[i]->Draw();

      if (show && mLoopSequence && i == mSequenceLoopEndIndex && mSequenceLoopEndIndex >= mSequenceLoopStartIndex)
      {
         ofPushStyle();
         ofRectangle upperRect = mSequencerSectionSelector[mSequenceLoopStartIndex]->GetRect(K(local));
         ofRectangle lowerRect = mSequencerSectionSelector[mSequenceLoopEndIndex]->GetRect(K(local));
         ofSetColor(150, 150, 150);
         ofSetLineWidth(1);
         ofLine(upperRect.getMinX() - 5, upperRect.getMinY(), lowerRect.getMinX() - 5, lowerRect.getMaxY());
         ofLine(upperRect.getMinX() - 5, upperRect.getMinY(), upperRect.getMinX() - 0, upperRect.getMinY());
         ofLine(lowerRect.getMinX() - 5, lowerRect.getMaxY(), lowerRect.getMinX() - 0, lowerRect.getMaxY());
         ofPopStyle();
      }

      if (mSequencerSectionId[i] < 0)
         sequenceComplete = true;
   }

   if (mSequenceStepIndex != -1)
   {
      ofPushStyle();
      ofFill();

      if (!mSequenceStartQueued)
      {
         ofSetColor(0, 255, 0);
         ofRectangle sectionEntryRect = mSequencerSectionSelector[mSequenceStepIndex]->GetRect(K(local));
         ofRect(sectionEntryRect.getMinX() - 5, sectionEntryRect.getCenter().y - 2, 4, 4);

         ofSetColor(0, 255, 0, 100);
         ofRectangle lengthEntryRect = mSequencerStepLengthEntry[mSequenceStepIndex]->GetRect(K(local));
         float progress = MIN((TheTransport->GetMeasurePos(NextBufferTime(false)) + mSequenceStepMeasureCount) / mSequencerStepLength[mSequenceStepIndex], 1.0f);
         lengthEntryRect.width *= progress;
         ofRect(lengthEntryRect);
      }

      if (mSequencePaused)
      {
         ofSetColor(0, 0, 255, 100);
         ofRectangle pauseButtonRect = mPauseSequenceButton->GetRect(K(local));
         ofRect(pauseButtonRect);
      }
      else
      {
         ofSetColor(0, 255, 0, 100);
         ofRectangle playButtonRect = mPlaySequenceButton->GetRect(K(local));
         ofRect(playButtonRect);
      }

      ofPopStyle();
   }
}

void SongBuilder::OnTimeEvent(double time)
{
   if (mQueuedSection != -1 && TheTransport->GetMeasure(time + TheTransport->GetListenerInfo(this)->mOffsetInfo.mOffset) % (int)TheTransport->GetMeasureFraction(mChangeQuantizeInterval) == 0)
   {
      SetActiveSection(time, mQueuedSection);
      mQueuedSection = -1;
   }

   if (mSequenceStepIndex != -1)
   {
      ++mSequenceStepMeasureCount;
      if (mSequenceStepMeasureCount >= mSequencerStepLength[mSequenceStepIndex] && !mSequencePaused)
      {
         mSequenceStepMeasureCount = 0;

         if (mLoopSequence && mSequenceStepIndex == mSequenceLoopEndIndex && mSequenceLoopEndIndex >= mSequenceLoopStartIndex)
            mSequenceStepIndex = mSequenceLoopStartIndex;
         else
            ++mSequenceStepIndex;

         if (mSequencerSectionId[mSequenceStepIndex] == kSequenceEndId)
         {
            mSequenceStepIndex = -1; //all done!
         }
         else
         {
            SetActiveSectionById(time, mSequencerSectionId[mSequenceStepIndex]);
            TheTransport->SetQueuedMeasure(0);
         }
      }
   }

   if (mSequenceStartQueued)
   {
      mSequenceStepIndex = mSequenceStartStepIndex;
      SetActiveSectionById(time, mSequencerSectionId[mSequenceStepIndex]);
      mSequenceStartQueued = false;
   }
}

void SongBuilder::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mCurrentSection < (int)mSections.size() - 1)
      SetActiveSection(time, mCurrentSection + 1);
}

void SongBuilder::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0 && pitch < (int)mSections.size())
      SetActiveSection(time, pitch);
}

void SongBuilder::SetActiveSectionById(double time, int newSectionId)
{
   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (mSections[i]->mId == newSectionId)
      {
         SetActiveSection(time, i);
         break;
      }
   }
}

void SongBuilder::SetActiveSection(double time, int newSection)
{
   if (newSection < (int)mSections.size())
   {
      for (int i = 0; i < (int)mTargets.size(); ++i)
      {
         for (auto& cable : mTargets[i]->mCable->GetPatchCables())
         {
            IUIControl* target = dynamic_cast<IUIControl*>(cable->GetTarget());
            if (target != nullptr)
            {
               if (mTargets[i]->mIsCheckbox)
                  target->SetValue(mSections[newSection]->mValues[i]->mBoolValue ? 1 : 0, time);
               else
                  target->SetValue(mSections[newSection]->mValues[i]->mValue, time);
            }
         }
      }
   }

   mCurrentSection = newSection;
}

void SongBuilder::RefreshSequencerDropdowns()
{
   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      mSequencerSectionSelector[i]->Clear();
      mSequencerSectionSelector[i]->AddLabel("-stop-", -1);
      for (auto* section : mSections)
         mSequencerSectionSelector[i]->AddLabel(section->mName, section->mId);
   }
}

void SongBuilder::GetModuleDimensions(float& width, float& height)
{
   width = kLeftMarginX + kSectionTabWidth + (int)mTargets.size() * (kColumnWidth + kSpacingX) + 3;
   if (ShowSongSequencer())
      width += kSongSequencerWidth;
   height = kGridStartY + kTargetTabHeightTop + kTargetTabHeightBottom + kSpacingY + (int)mSections.size() * (kRowHeight + kSpacingY) + 3;

   if (ShowSongSequencer())
   {
      for (int i = 0; i < kMaxSequencerSections; ++i)
      {
         if (i == kMaxSequencerSections - 1 || mSequencerSectionId[i] < 0)  //end of sequence
         {
            ofRectangle rect = mSequencerSectionSelector[i]->GetRect(K(local));
            if (rect.getMaxY() + 3 > height)
               height = rect.getMaxY() + 3;
            break;
         }
      }
   }
}

void SongBuilder::PostRepatch(PatchCableSource* cable, bool fromUserClick)
{
   int targetIndex = -1;
   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (cable == mTargets[i]->mCable)
      {
         mTargets[i]->TargetControlUpdated();
         targetIndex = i;
         break;
      }
   }

   if (fromUserClick && targetIndex != -1)
   {
      for (int i = 0; i < (int)mSections.size(); ++i)
         mSections[i]->TargetControlUpdated(mTargets[targetIndex], targetIndex, true);

      if (mTargets[targetIndex]->GetTarget() == nullptr)
      {
         mTargets[targetIndex]->CleanUp();
         mTargets.erase(mTargets.begin() + targetIndex);
      }
   }
}

void SongBuilder::PlaySequence(int startIndex)
{
   if (mSequencePaused && startIndex == -1)
   {
      mSequencePaused = false;
   }
   else
   {
      if (startIndex == -1)
         mSequenceStartStepIndex = 0;
      else
         mSequenceStartStepIndex = startIndex;
      mSequenceStepMeasureCount = 0;
      mSequencePaused = false;
      mSequenceStartQueued = true;
      TheTransport->SetQueuedMeasure(0);
   }
}

void SongBuilder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPlaySequenceButton)
      PlaySequence(-1);

   if (button == mStopSequenceButton)
   {
      mSequenceStartQueued = false;
      mSequenceStepIndex = -1;
      mSequencePaused = false;
   }

   if (button == mPauseSequenceButton)
   {
      if (mSequencePaused)
      {
         mSequencePaused = false;
      }
      else if (mSequenceStepIndex != -1)
      {
         mSequencePaused = true;
         mSequenceStepMeasureCount = mSequencerStepLength[mSequenceStepIndex];
      }
   }

   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (button == mSections[i]->mActivateButton)
      {
         if (mChangeQuantizeInterval == kInterval_None)
            SetActiveSection(time, i);
         else
            mQueuedSection = i;
      }
   }

   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (button == mTargets[i]->mMoveLeftButton)
      {
         if (i > 0)
         {
            ControlTarget* target = mTargets[i];
            mTargets.erase(mTargets.begin() + i);
            mTargets.insert(mTargets.begin() + (i - 1), target);
            for (auto* section : mSections)
               section->MoveValue(i, -1);
            gHoveredUIControl = nullptr;
         }
         break;
      }
      if (button == mTargets[i]->mMoveRightButton)
      {
         if (i < (int)mTargets.size() - 1)
         {
            ControlTarget* target = mTargets[i];
            mTargets.erase(mTargets.begin() + i);
            mTargets.insert(mTargets.begin() + (i + 1), target);
            for (auto* section : mSections)
               section->MoveValue(i, 1);
            gHoveredUIControl = nullptr;
         }
         break;
      }
   }

   if (button == mAddTargetButton)
      AddTarget();

   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      if (button == mSequencerPlayFromButton[i])
         PlaySequence(i);
   }
}

void SongBuilder::TextEntryComplete(TextEntry* entry)
{
   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (entry == mSections[i]->mNameEntry)
         RefreshSequencerDropdowns();
   }
}

void SongBuilder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   for (int i = 0; i < (int)mSections.size(); ++i)
   {
      if (list == mSections[i]->mContextMenu)
      {
         switch (mSections[i]->mContextMenuSelection)
         {
            case ContextMenuItems::kDuplicate:
               DuplicateSection(i);
               break;
            case ContextMenuItems::kDelete:
               if (mSections.size() > 1)
               {
                  mSections[i]->CleanUp();
                  mSections.erase(mSections.begin() + i);
                  RefreshSequencerDropdowns();
                  i = -1;
               }
               break;
            case ContextMenuItems::kMoveUp:
               if (i > 0)
               {
                  SongSection* section = mSections[i];
                  mSections.erase(mSections.begin() + i);
                  --i;
                  mSections.insert(mSections.begin() + i, section);
                  RefreshSequencerDropdowns();
               }
               break;
            case ContextMenuItems::kMoveDown:
               if (i < (int)mSections.size() - 1)
               {
                  SongSection* section = mSections[i];
                  mSections.erase(mSections.begin() + i);
                  ++i;
                  mSections.insert(mSections.begin() + i, section);
                  RefreshSequencerDropdowns();
               }
               break;
            case ContextMenuItems::kNone:
               break;
         }

         if (i != -1)
            mSections[i]->mContextMenuSelection = ContextMenuItems::kNone;

         break;
      }
   }

   for (int i = 0; i < kMaxSequencerSections; ++i)
   {
      if (list == mSequencerContextMenu[i])
      {
         switch (mSequencerContextMenuSelection[i])
         {
            case ContextMenuItems::kDuplicate:
               for (int j = kMaxSequencerSections - 1; j > i; --j)
               {
                  mSequencerSectionId[j] = mSequencerSectionId[j - 1];
                  mSequencerStepLength[j] = mSequencerStepLength[j - 1];
               }
               break;
            case ContextMenuItems::kDelete:
               for (int j = i; j < kMaxSequencerSections - 1; ++j)
               {
                  mSequencerSectionId[j] = mSequencerSectionId[j + 1];
                  mSequencerStepLength[j] = mSequencerStepLength[j + 1];
               }
               break;
            case ContextMenuItems::kMoveUp:
               if (i > 0)
               {
                  int temp = mSequencerSectionId[i];
                  mSequencerSectionId[i] = mSequencerSectionId[i - 1];
                  mSequencerSectionId[i - 1] = temp;

                  temp = mSequencerStepLength[i];
                  mSequencerStepLength[i] = mSequencerStepLength[i - 1];
                  mSequencerStepLength[i - 1] = temp;
               }
               break;
            case ContextMenuItems::kMoveDown:
               if (i < kMaxSequencerSections - 1 && mSequencerSectionId[i + 1] >= 0)
               {
                  int temp = mSequencerSectionId[i];
                  mSequencerSectionId[i] = mSequencerSectionId[i + 1];
                  mSequencerSectionId[i + 1] = temp;

                  temp = mSequencerStepLength[i];
                  mSequencerStepLength[i] = mSequencerStepLength[i + 1];
                  mSequencerStepLength[i + 1] = temp;
               }
               break;
            case ContextMenuItems::kNone:
               break;
         }

         mSequencerContextMenuSelection[i] = ContextMenuItems::kNone;

         break;
      }
   }
}

void SongBuilder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SongBuilder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   if (IsSpawningOnTheFly(moduleInfo))
   {
      mSections.push_back(new SongSection("start"));
      mSections.push_back(new SongSection("intro"));
      mSections.push_back(new SongSection("verse"));
      mSections.push_back(new SongSection("chorus"));
      mSections.push_back(new SongSection("bridge"));
      mSections.push_back(new SongSection("outro"));
      mSections.push_back(new SongSection("done"));
      for (auto* section : mSections)
         section->CreateUIControls(this);

      RefreshSequencerDropdowns();
   }

   SetUpFromSaveData();
}

void SongBuilder::SetUpFromSaveData()
{
}

void SongBuilder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   out << (int)mTargets.size();
   for (auto* target : mTargets)
   {
      target->mCable->SaveState(out);
      out << target->mIsCheckbox;
   }

   out << (int)mSections.size();
   for (auto* section : mSections)
   {
      out << section->mName;
      out << section->mId;
      out << (int)section->mValues.size();
      for (auto* value : section->mValues)
      {
         out << value->mId;
         out << value->mValue;
         out << value->mBoolValue;
      }
   }

   IDrawableModule::SaveState(out);
}

void SongBuilder::LoadState(FileStreamIn& in, int rev)
{
   int numTargets;
   in >> numTargets;
   mTargets.resize(numTargets);
   for (int i = 0; i < numTargets; ++i)
   {
      mTargets[i] = new ControlTarget();
      mTargets[i]->CreateUIControls(this);
      mTargets[i]->mCable->LoadState(in);
      in >> mTargets[i]->mIsCheckbox;
   }

   for (auto* section : mSections)
      section->CleanUp();

   int numSections;
   in >> numSections;
   mSections.resize(numSections);
   for (int i = 0; i < numSections; ++i)
   {
      mSections[i] = new SongSection("");
      in >> mSections[i]->mName;
      in >> mSections[i]->mId;
      mSections[i]->CreateUIControls(this);

      int numValues;
      in >> numValues;
      mSections[i]->mValues.resize(numValues);
      for (int j = 0; j < numValues; ++j)
      {
         mSections[i]->mValues[j] = new ControlValue();
         in >> mSections[i]->mValues[j]->mId;
         in >> mSections[i]->mValues[j]->mValue;
         in >> mSections[i]->mValues[j]->mBoolValue;
         mSections[i]->mValues[j]->CreateUIControls(this);
         mSections[i]->TargetControlUpdated(mTargets[j], j, false);
      }
   }

   RefreshSequencerDropdowns();

   IDrawableModule::LoadState(in, rev);
}

void SongBuilder::DuplicateSection(int sectionIndex)
{
   std::vector<std::string> sectionNames;
   for (auto* section : mSections)
      sectionNames.push_back(section->mName);
   std::string numberless = mSections[sectionIndex]->mName;
   while (numberless.size() > 1 && isdigit(numberless[numberless.size() - 1]))
      numberless = numberless.substr(0, numberless.size() - 1);
   std::string newSectionName = GetUniqueName(numberless, sectionNames);
   SongSection* newSection = new SongSection(newSectionName);
   newSection->CreateUIControls(this);
   mSections.insert(mSections.begin() + sectionIndex + 1, newSection);
   for (auto* value : mSections[sectionIndex]->mValues)
   {
      newSection->AddValue(this);
      auto* newValue = newSection->mValues[newSection->mValues.size() - 1];
      newValue->mValue = value->mValue;
      newValue->mBoolValue = value->mBoolValue;
   }

   RefreshSequencerDropdowns();
}

void SongBuilder::AddTarget()
{
   ControlTarget* target = new ControlTarget();
   target->CreateUIControls(this);
   mTargets.push_back(target);

   for (int i = 0; i < (int)mSections.size(); ++i)
      mSections[i]->AddValue(this);
}

void SongBuilder::SongSection::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* section : owner->mSections)
      {
         if (section != this && mId <= section->mId)
            mId = section->mId + 1;
      }
   }

#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER owner //change owner
   UIBLOCK0();
   BUTTON_STYLE(mActivateButton, ("go" + ofToString(mId)).c_str(), ButtonDisplayStyle::kPlay);
   TEXTENTRY(mNameEntry, ("name" + ofToString(mId)).c_str(), 12, &mName);
   DROPDOWN(mContextMenu, ("context " + ofToString(mId)).c_str(), (int*)(&mContextMenuSelection), 20);
   ENDUIBLOCK0();
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset

   mContextMenu->AddLabel("duplicate", (int)ContextMenuItems::kDuplicate);
   mContextMenu->AddLabel("delete", (int)ContextMenuItems::kDelete);
   mContextMenu->AddLabel("move up", (int)ContextMenuItems::kMoveUp);
   mContextMenu->AddLabel("move down", (int)ContextMenuItems::kMoveDown);
   mContextMenu->SetDisplayStyle(DropdownDisplayStyle::kHamburger);
}

void SongBuilder::SongSection::AddValue(SongBuilder* owner)
{
   ControlValue* value = new ControlValue();
   value->CreateUIControls(owner);
   int index = (int)mValues.size();
   mValues.push_back(value);
   TargetControlUpdated(owner->mTargets[index], index, false);
}

void SongBuilder::SongSection::TargetControlUpdated(SongBuilder::ControlTarget* target, int targetIndex, bool wasManuallyPatched)
{
   IUIControl* control = target->GetTarget();
   if (control != nullptr)
   {
      if (target->mIsCheckbox)
         mValues[targetIndex]->mBoolValue = control->GetValue() > 0;
      else
         mValues[targetIndex]->mValue = control->GetValue();

      mValues[targetIndex]->mValueEntry->SetShowing(!target->mIsCheckbox);
      mValues[targetIndex]->mCheckbox->SetShowing(target->mIsCheckbox);
   }
   else if (wasManuallyPatched) //user intentionally deleted connection
   {
      mValues[targetIndex]->CleanUp();
      mValues.erase(mValues.begin() + targetIndex);
   }
   else
   {
      mValues[targetIndex]->mValueEntry->SetShowing(false);
      mValues[targetIndex]->mCheckbox->SetShowing(false);
   }
}

void SongBuilder::SongSection::Draw(SongBuilder* owner, float x, float y, int sectionIndex)
{
   float width = GetWidth();
   float height = kRowHeight;
   ofPushStyle();
   ofNoFill();
   ofSetColor(ofColor(150, 150, 150));
   ofRect(x, y, width, height);
   if (sectionIndex == owner->mCurrentSection)
   {
      ofFill();
      ofSetColor(ofColor(130, 130, 130, 130));
      ofRect(x, y, width, height);
   }
   if (sectionIndex == owner->mQueuedSection)
   {
      ofFill();
      ofSetColor(ofColor(0, 130, 0, ofMap(sin(TheTransport->GetMeasurePos(gTime) * TWO_PI * 4), -1, 1, 50, 100)));
      ofRect(x, y, width, height);
   }
   ofPopStyle();
   mActivateButton->SetPosition(x + 3, y + 3);
   mActivateButton->Draw();
   mNameEntry->PositionTo(mActivateButton, kAnchor_Right);
   mNameEntry->Draw();
   mContextMenu->PositionTo(mNameEntry, kAnchor_Right);
   mContextMenu->Draw();

   for (int i = 0; i < (int)mValues.size(); ++i)
      mValues[i]->Draw(x + kSectionTabWidth + i * (kColumnWidth + kSpacingX), y, sectionIndex, i);
}

void SongBuilder::SongSection::MoveValue(int index, int amount)
{
   if (index + amount >= 0 && index + amount < (int)mValues.size())
   {
      ControlValue* value = mValues[index];
      mValues.erase(mValues.begin() + index);
      index += amount;
      mValues.insert(mValues.begin() + index, value);
   }
}

float SongBuilder::SongSection::GetWidth() const
{
   return kSectionTabWidth + (int)mValues.size() * (kColumnWidth + kSpacingX);
}

void SongBuilder::SongSection::CleanUp()
{
   mNameEntry->RemoveFromOwner();
   mActivateButton->RemoveFromOwner();
   mContextMenu->RemoveFromOwner();
   for (auto& value : mValues)
      value->CleanUp();
}

void SongBuilder::ControlTarget::CreateUIControls(SongBuilder* owner)
{
   mCable = new PatchCableSource(owner, kConnectionType_UIControl);
   owner->AddPatchCableSource(mCable);
   mCable->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   mMoveLeftButton = new ClickButton(owner, "", -1, -1, ButtonDisplayStyle::kArrowLeft);
   mMoveRightButton = new ClickButton(owner, "", -1, -1, ButtonDisplayStyle::kArrowRight);
}

void SongBuilder::ControlTarget::Draw(float x, float y, int numRows)
{
   ofPushStyle();
   ofFill();
   ofSetColor(ofColor(130, 130, 130, 130));
   ofRect(x, y, kColumnWidth, kTargetTabHeightTop);
   ofRect(x, y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY), kColumnWidth, kTargetTabHeightBottom);
   ofPopStyle();

   IUIControl* target = GetTarget();
   if (target)
   {
      ofPushMatrix();
      ofClipWindow(x, y, kColumnWidth, kTargetTabHeightTop, true);
      std::string text = target->Path(false, true);
      if (mIsCheckbox)
         ofStringReplace(text, "~enabled", "");
      std::string displayString;
      const int kSliceSize = 11;
      int cursor = 0;
      for (; cursor + kSliceSize < (int)text.size(); cursor += kSliceSize)
         displayString += text.substr(cursor, kSliceSize) + "\n";
      displayString += text.substr(cursor, (int)text.size() - cursor);
      DrawTextNormal(displayString, x + 2, y + 9, 9);
      ofPopMatrix();
   }
   float bottomY = y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY);
   mCable->SetManualPosition(x + kColumnWidth * .5f, bottomY + 5);
   mMoveLeftButton->SetPosition(x, bottomY - 3);
   if (gHoveredUIControl == mMoveLeftButton)
      mMoveLeftButton->Draw();
   mMoveRightButton->SetPosition(x + kColumnWidth - 20, bottomY - 3);
   if (gHoveredUIControl == mMoveRightButton)
      mMoveRightButton->Draw();
}

IUIControl* SongBuilder::ControlTarget::GetTarget() const
{
   IUIControl* target = nullptr;
   if (!mCable->GetPatchCables().empty())
      target = dynamic_cast<IUIControl*>(mCable->GetPatchCables()[0]->GetTarget());
   return target;
}

void SongBuilder::ControlTarget::TargetControlUpdated()
{
   IUIControl* target = GetTarget();
   mIsCheckbox = (dynamic_cast<Checkbox*>(target) != nullptr || dynamic_cast<ClickButton*>(target) != nullptr);
}

void SongBuilder::ControlTarget::CleanUp()
{
   mCable->GetOwner()->RemovePatchCableSource(mCable);
   mMoveLeftButton->RemoveFromOwner();
   mMoveRightButton->RemoveFromOwner();
}

void SongBuilder::ControlValue::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* section : owner->mSections)
      {
         for (auto* value : section->mValues)
         {
            if (value != this && mId <= value->mId)
               mId = value->mId + 1;
         }
      }
   }

   mValueEntry = new TextEntry(owner, ("value " + ofToString(mId)).c_str(), -1, -1, 4, &mValue, -9999, 9999);
   mCheckbox = new Checkbox(owner, ("checkbox " + ofToString(mId)).c_str(), -1, -1, &mBoolValue);
   mCheckbox->SetDisplayText(false);
}

void SongBuilder::ControlValue::Draw(float x, float y, int sectionIndex, int targetIndex)
{
   ofPushStyle();
   ofFill();
   ofSetColor(ofColor(130, 130, 130, 130));
   ofRect(x, y + 2, kColumnWidth, kRowHeight - 4);
   ofPopStyle();

   mValueEntry->SetPosition(x + 7, y + 3);
   mValueEntry->Draw();
   mCheckbox->SetPosition(x + 20, y + 3);
   mCheckbox->Draw();
}

void SongBuilder::ControlValue::CleanUp()
{
   mValueEntry->RemoveFromOwner();
   mCheckbox->RemoveFromOwner();
}