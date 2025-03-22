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
#include "RadioButton.h"

namespace
{
   const float kLeftMarginX = 3;
   const float kSongSequencerWidth = 175;
   const float kGridStartY = 20;
   const float kSceneTabWidth = 165;
   const float kTargetTabHeightTop = 30;
   const float kTargetTabHeightBottom = 10;
   const float kRowHeight = 20;
   const float kColumnWidth = 50;
   const float kSpacingX = 3;
   const float kSpacingY = 3;
}

SongBuilder::SongBuilder()
{
   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      mSequencerSceneId[i] = -1;
      mSequencerSceneSelector[i] = nullptr;
      mSequencerStepLength[i] = 4;
      mSequencerStepLengthEntry[i] = nullptr;
      mSequencerContextMenu[i] = nullptr;
      mSequencerContextMenuSelection[i] = ContextMenuItems::kNone;
   }

   const float kColorDim = .7f;
   ofColor grey = IDrawableModule::GetColor(kModuleCategory_Other);
   mColors.push_back(TargetColor("grey", grey * kColorDim));
   mColors.push_back(TargetColor("red", ofColor::red * kColorDim));
   mColors.push_back(TargetColor("orange", ofColor::orange * kColorDim));
   mColors.push_back(TargetColor("yellow", ofColor::yellow * kColorDim));
   mColors.push_back(TargetColor("green", ofColor::green * kColorDim));
   mColors.push_back(TargetColor("cyan", ofColor::cyan * kColorDim));
   mColors.push_back(TargetColor("blue", ofColor::blue * kColorDim));
   mColors.push_back(TargetColor("purple", ofColor::purple * kColorDim));
   mColors.push_back(TargetColor("magenta", ofColor::magenta * kColorDim));

   mTransportPriority = ITimeListener::kTransportPriorityVeryEarly;
}

void SongBuilder::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(gBufferSizeMs, true), true);

   SetActiveScene(gTime, 0);
}

void SongBuilder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float width, height;
   UIBLOCK0();
   CHECKBOX(mUseSequencerCheckbox, "use sequencer", &mUseSequencer);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mActivateFirstSceneOnStopCheckbox, "activate first scene on stop", &mActivateFirstSceneOnStop);
   UIBLOCK_NEWLINE();
   BUTTON_STYLE(mPlaySequenceButton, "play", ButtonDisplayStyle::kPlay);
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mStopSequenceButton, "stop", ButtonDisplayStyle::kStop);
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mPauseSequenceButton, "pause", ButtonDisplayStyle::kPause);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLoopCheckbox, "loop", &mLoopSequence);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY_NUM(mSequenceLoopStartEntry, "loop start", 3, &mSequenceLoopStartIndex, 0, kMaxSequencerScenes - 1);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY_NUM(mSequenceLoopEndEntry, "loop end", 3, &mSequenceLoopEndIndex, 0, kMaxSequencerScenes - 1);
   ENDUIBLOCK(width, height);

   UIBLOCK(10, height + 4);
   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      DROPDOWN(mSequencerSceneSelector[i], ("scene" + ofToString(i)).c_str(), &mSequencerSceneId[i], 80);
      mSequencerSceneSelector[i]->SetCableTargetable(false);
      mSequencerSceneSelector[i]->SetDrawTriangle(false);
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
      mSequencerContextMenu[i]->SetCableTargetable(false);
      mSequencerContextMenu[i]->SetDisplayStyle(DropdownDisplayStyle::kHamburger);
      UIBLOCK_NEWLINE();
   }
   ENDUIBLOCK0();

   mChangeQuantizeSelector = new DropdownList(this, "change quantize", -1, -1, (int*)(&mChangeQuantizeInterval));
   mAddTargetButton = new ClickButton(this, "add target", -1, -1, ButtonDisplayStyle::kPlus);
   mAddTargetButton->SetCableTargetable(false);

   mChangeQuantizeSelector->AddLabel("jump", kInterval_None);
   mChangeQuantizeSelector->AddLabel("switch", kInterval_Free);
   mChangeQuantizeSelector->AddLabel("1n", kInterval_1n);
   mChangeQuantizeSelector->AddLabel("2", kInterval_2);
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
   mActivateFirstSceneOnStopCheckbox->SetShowing(ShowSongSequencer());
   mActivateFirstSceneOnStopCheckbox->SetPosition(gridStartX, 3);
   mActivateFirstSceneOnStopCheckbox->Draw();
   mChangeQuantizeSelector->SetPosition(gridStartX, kGridStartY + kTargetTabHeightTop - 29);
   mChangeQuantizeSelector->Draw();
   mAddTargetButton->SetPosition(gridStartX + kSceneTabWidth - 22, kGridStartY + 8);
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

   //separator
   if (ShowSongSequencer())
      ofLine(gridStartX - 8, kGridStartY + kTargetTabHeightTop + kSpacingY, gridStartX - 8, kGridStartY + kTargetTabHeightTop + (int)mScenes.size() * (kRowHeight + kSpacingY));

   DrawTextNormal("scenes:", gridStartX, kGridStartY + kTargetTabHeightTop - 1);

   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      if (mScenes[i] != nullptr)
         mScenes[i]->Draw(this, gridStartX, kGridStartY + kTargetTabHeightTop + kSpacingY + i * (kRowHeight + kSpacingY), i);
   }

   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (mTargets[i] != nullptr)
         mTargets[i]->Draw(gridStartX + kSceneTabWidth + i * (kColumnWidth + kSpacingX), kGridStartY, (int)mScenes.size());
   }

   bool sequenceComplete = false;
   int sequenceLength = 0;
   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      bool show = mUseSequencer && !sequenceComplete;
      mSequencerSceneSelector[i]->SetShowing(show);
      mSequencerStepLengthEntry[i]->SetShowing(show && mSequencerSceneId[i] != kSequenceEndId);
      mSequencerContextMenu[i]->SetShowing(show && mSequencerSceneId[i] >= 0);
      mSequencerPlayFromButton[i]->SetShowing(show && mSequencerSceneId[i] >= 0);
      mSequencerSceneSelector[i]->Draw();
      mSequencerStepLengthEntry[i]->Draw();
      mSequencerContextMenu[i]->Draw();
      mSequencerPlayFromButton[i]->Draw();

      if (show && mLoopSequence && i == mSequenceLoopEndIndex && mSequenceLoopEndIndex >= mSequenceLoopStartIndex)
      {
         ofPushStyle();
         ofRectangle upperRect = mSequencerSceneSelector[mSequenceLoopStartIndex]->GetRect(K(local));
         ofRectangle lowerRect = mSequencerSceneSelector[mSequenceLoopEndIndex]->GetRect(K(local));
         ofSetColor(150, 150, 150);
         ofSetLineWidth(1);
         ofLine(upperRect.getMinX() - 5, upperRect.getMinY(), lowerRect.getMinX() - 5, lowerRect.getMaxY());
         ofLine(upperRect.getMinX() - 5, upperRect.getMinY(), upperRect.getMinX() - 0, upperRect.getMinY());
         ofLine(lowerRect.getMinX() - 5, lowerRect.getMaxY(), lowerRect.getMinX() - 0, lowerRect.getMaxY());
         ofPopStyle();
      }

      if (mSequencerSceneId[i] < 0)
      {
         if (show && !sequenceComplete)
         {
            ofRectangle rect = mSequencerStepLengthEntry[i]->GetRect(K(local));
            int sequenceLengthSeconds = int(sequenceLength * TheTransport->MsPerBar() / 1000);
            int sequenceLengthMinutes = sequenceLengthSeconds / 60;
            sequenceLengthSeconds %= 60;
            std::string lengthTime = ofToString(sequenceLengthMinutes) + ":";
            if (sequenceLengthSeconds < 10)
               lengthTime += "0"; //zero pad
            lengthTime += ofToString(sequenceLengthSeconds);
            DrawTextNormal(ofToString(sequenceLength) + " (" + lengthTime + ")", rect.x, rect.y + 12);
         }
         sequenceComplete = true;
      }
      else if (!sequenceComplete)
      {
         sequenceLength += mSequencerStepLength[i];
      }
   }

   if (ShowSongSequencer() && mSequenceStepIndex != -1)
   {
      ofPushStyle();
      ofFill();

      if (!mSequenceStartQueued)
      {
         ofSetColor(0, 255, 0);
         ofRectangle sceneEntryRect = mSequencerSceneSelector[mSequenceStepIndex]->GetRect(K(local));
         ofRect(sceneEntryRect.getMinX() - 5, sceneEntryRect.getCenter().y - 2, 4, 4);

         ofSetColor(0, 255, 0, 100);
         ofRectangle lengthEntryRect = mSequencerStepLengthEntry[mSequenceStepIndex]->GetRect(K(local));
         float progress = MIN(TheTransport->GetMeasureTime(gTime) / mSequencerStepLength[mSequenceStepIndex], 1.0f);
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

void SongBuilder::Poll()
{
   if (mWantRefreshValueDropdowns)
   {
      for (int i = 0; i < (int)mScenes.size(); ++i)
      {
         for (int j = 0; j < (int)mTargets.size(); ++j)
            mScenes[i]->mValues[j]->UpdateDropdownContents(mTargets[j]);
      }
      mWantRefreshValueDropdowns = false;
   }
}

void SongBuilder::OnTimeEvent(double time)
{
   if (mJustResetClock)
   {
      mJustResetClock = false;
      return;
   }

   if (mQueuedScene != -1 &&
       (mChangeQuantizeInterval == kInterval_None ||
        mChangeQuantizeInterval == kInterval_Free ||
        TheTransport->GetMeasure(time + TheTransport->GetListenerInfo(this)->mOffsetInfo.mOffset) % (int)TheTransport->GetMeasureFraction(mChangeQuantizeInterval) == 0))
   {
      SetActiveScene(time, mQueuedScene);
      mQueuedScene = -1;
   }

   if (mSequenceStepIndex != -1)
   {
      if (TheTransport->GetMeasure(time + TheTransport->GetListenerInfo(this)->mOffsetInfo.mOffset) >= mSequencerStepLength[mSequenceStepIndex] && !mSequencePaused)
      {
         if (mLoopSequence && mSequenceStepIndex == mSequenceLoopEndIndex && mSequenceLoopEndIndex >= mSequenceLoopStartIndex)
            mSequenceStepIndex = mSequenceLoopStartIndex;
         else
            ++mSequenceStepIndex;

         if (mSequencerSceneId[mSequenceStepIndex] == kSequenceEndId)
         {
            mSequenceStepIndex = -1; //all done!

            if (mActivateFirstSceneOnStop)
               SetActiveScene(time, 0);
         }
         else
         {
            SetActiveSceneById(time, mSequencerSceneId[mSequenceStepIndex]);
            if (mResetOnSceneChange)
               mWantResetClock = true;
         }
      }
   }

   if (mSequenceStartQueued)
   {
      mSequenceStepIndex = mSequenceStartStepIndex;
      SetActiveSceneById(time, mSequencerSceneId[mSequenceStepIndex]);
      mSequenceStartQueued = false;
      mWantResetClock = true;
   }

   if (mWantResetClock)
   {
      TheTransport->SetQueuedMeasure(time, 0);
      mWantResetClock = false;
      mJustResetClock = true;
   }
}

void SongBuilder::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mCurrentScene < (int)mScenes.size() - 1)
      SetActiveScene(time, mCurrentScene + 1);
}

void SongBuilder::PlayNote(NoteMessage note)
{
   if (note.velocity > 0 && note.pitch < (int)mScenes.size())
      SetActiveScene(note.time, note.pitch);
}

void SongBuilder::SetActiveSceneById(double time, int newSceneId)
{
   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      if (mScenes[i]->mId == newSceneId)
      {
         SetActiveScene(time, i);
         break;
      }
   }
}

void SongBuilder::SetActiveScene(double time, int newScene)
{
   if (newScene < (int)mScenes.size())
   {
      for (int i = 0; i < (int)mTargets.size(); ++i)
      {
         for (auto& cable : mTargets[i]->mCable->GetPatchCables())
         {
            IUIControl* target = dynamic_cast<IUIControl*>(cable->GetTarget());
            if (target != nullptr)
            {
               if (mTargets[i]->mDisplayType == ControlTarget::DisplayType::TextEntry)
                  target->SetValue(mScenes[newScene]->mValues[i]->mFloatValue, time);
               if (mTargets[i]->mDisplayType == ControlTarget::DisplayType::Checkbox)
                  target->SetValue(mScenes[newScene]->mValues[i]->mBoolValue ? 1 : 0, time);
               if (mTargets[i]->mDisplayType == ControlTarget::DisplayType::Dropdown)
                  target->SetValue(mScenes[newScene]->mValues[i]->mIntValue, time);
            }
         }
      }
   }

   mCurrentScene = newScene;
}

void SongBuilder::RefreshSequencerDropdowns()
{
   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      mSequencerSceneSelector[i]->Clear();
      mSequencerSceneSelector[i]->AddLabel("-stop-", kSequenceEndId);
      for (auto* scene : mScenes)
         mSequencerSceneSelector[i]->AddLabel(scene->mName, scene->mId);
   }
}

void SongBuilder::GetModuleDimensions(float& width, float& height)
{
   width = kLeftMarginX + kSceneTabWidth + (int)mTargets.size() * (kColumnWidth + kSpacingX) + 3;
   if (ShowSongSequencer())
      width += kSongSequencerWidth;
   height = kGridStartY + kTargetTabHeightTop + kTargetTabHeightBottom + kSpacingY + (int)mScenes.size() * (kRowHeight + kSpacingY) + 3;

   if (ShowSongSequencer())
   {
      for (int i = 0; i < kMaxSequencerScenes; ++i)
      {
         if (i == kMaxSequencerScenes - 1 || mSequencerSceneId[i] < 0) //end of sequence
         {
            ofRectangle rect = mSequencerSceneSelector[i]->GetRect(K(local));
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

   if (targetIndex != -1)
   {
      if (fromUserClick)
      {
         for (int i = 0; i < (int)mScenes.size(); ++i)
            mScenes[i]->TargetControlUpdated(mTargets[targetIndex], targetIndex, true);

         if (mTargets[targetIndex]->GetTarget() == nullptr)
         {
            mTargets[targetIndex]->CleanUp();
            mTargets.erase(mTargets.begin() + targetIndex);
         }
      }
      if (!mTargets.empty() && mTargets.size() > targetIndex)
         mTargets[targetIndex]->mHadTarget = (mTargets[targetIndex]->GetTarget() != nullptr);
   }
}

void SongBuilder::PlaySequence(double time, int startIndex)
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
      mWantResetClock = true;
      mSequencePaused = false;
      mSequenceStartQueued = true;
   }
}

bool SongBuilder::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99 && midiValue > 0)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;

         if (x == 0)
         {
            if (mUseSequencer)
            {
               switch (y)
               {
                  case 0: mPlaySequenceButton->SetValue(1, gTime); break;
                  case 1: mStopSequenceButton->SetValue(1, gTime); break;
                  default: break;
               }
            }
         }
         else
         {
            int index = y + (x - 1) * 8;
            if (index < mScenes.size())
               mScenes[index]->mActivateButton->SetValue(1, gTime);
         }

         return true;
      }
   }

   return false;
}

void SongBuilder::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor = 0;
         int pushColorBlink = -1;

         if (x == 0)
         {
            if (mUseSequencer)
            {
               switch (y)
               {
                  case 0:
                     pushColor = mSequenceStepIndex == -1 ? 86 : 126;
                     if (mSequenceStartQueued)
                        pushColorBlink = 0;
                     break;
                  case 1:
                     pushColor = mSequenceStepIndex == -1 ? 127 : 68;
                     break;
                  default: break;
               }
            }
         }
         else
         {
            int index = y + (x - 1) * 8;
            if (index < mScenes.size())
            {
               if (index == mCurrentScene)
                  pushColor = 120;
               else
                  pushColor = 124;

               if (index == mQueuedScene)
                  pushColorBlink = 0;
            }
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor, pushColorBlink);
      }
   }
}

bool SongBuilder::DrawToPush2Screen()
{
   ofPushStyle();
   ofSetColor(255, 255, 255);
   if (mQueuedScene >= 0 && mQueuedScene < mScenes.size())
      DrawTextNormal("queued: " + mScenes[mQueuedScene]->mName, 100, -2);
   else if (mCurrentScene >= 0 && mCurrentScene < mScenes.size())
      DrawTextNormal("scene: " + mScenes[mCurrentScene]->mName, 100, -2);
   ofPopStyle();

   return false;
}

void SongBuilder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPlaySequenceButton)
   {
      PlaySequence(time, -1);
      if (mChangeQuantizeInterval == kInterval_None) //jump
         TheTransport->Reset();
   }

   if (button == mStopSequenceButton)
   {
      mSequenceStartQueued = false;
      mSequenceStepIndex = -1;
      mSequencePaused = false;

      if (mActivateFirstSceneOnStop)
         SetActiveScene(time, 0);
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
      }
   }

   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      if (button == mScenes[i]->mActivateButton)
      {
         mSequenceStepIndex = -1; //stop playing
         if (mChangeQuantizeInterval == kInterval_Free) //switch
         {
            SetActiveScene(time, i);
         }
         else if (mChangeQuantizeInterval == kInterval_None) //jump
         {
            mQueuedScene = i;
            TheTransport->Reset();
         }
         else
         {
            mQueuedScene = i;
         }
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
            for (auto* scene : mScenes)
               scene->MoveValue(i, -1);
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
            for (auto* scene : mScenes)
               scene->MoveValue(i, 1);
            gHoveredUIControl = nullptr;
         }
         break;
      }
      if (button == mTargets[i]->mCycleDisplayTypeButton)
      {
         while (true)
         {
            mTargets[i]->mDisplayType = (ControlTarget::DisplayType)(((int)mTargets[i]->mDisplayType + 1) % (int)ControlTarget::DisplayType::NumDisplayTypes);
            if (mTargets[i]->mDisplayType == ControlTarget::DisplayType::Dropdown && dynamic_cast<DropdownList*>(mTargets[i]->GetTarget()) == nullptr && dynamic_cast<RadioButton*>(mTargets[i]->GetTarget()) == nullptr)
               continue; //invalid option
            else
               break;
         }
         break;
      }
   }

   if (button == mAddTargetButton)
      AddTarget();

   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      if (button == mSequencerPlayFromButton[i])
      {
         PlaySequence(time, i);
         if (mChangeQuantizeInterval == kInterval_None) //jump
            TheTransport->Reset();
      }
   }
}

void SongBuilder::TextEntryComplete(TextEntry* entry)
{
   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      if (entry == mScenes[i]->mNameEntry)
         RefreshSequencerDropdowns();
   }
}

void SongBuilder::DropdownClicked(DropdownList* list)
{
   int refreshValueColumn = -1;
   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      for (int j = 0; j < (int)mScenes[i]->mValues.size(); ++j)
      {
         if (list == mScenes[i]->mValues[j]->mValueSelector && mTargets[j]->GetTarget() != nullptr && mTargets[j]->mDisplayType == ControlTarget::DisplayType::Dropdown)
            refreshValueColumn = j;
      }
   }

   if (refreshValueColumn != -1)
   {
      for (int i = 0; i < (int)mScenes.size(); ++i)
         mScenes[i]->mValues[refreshValueColumn]->UpdateDropdownContents(mTargets[refreshValueColumn]);
   }
}

void SongBuilder::ControlValue::UpdateDropdownContents(ControlTarget* target)
{
   if (target->mDisplayType == ControlTarget::DisplayType::Dropdown)
   {
      DropdownList* targetList = dynamic_cast<DropdownList*>(target->GetTarget());
      if (targetList)
         targetList->CopyContentsTo(mValueSelector);
      RadioButton* targetRadio = dynamic_cast<RadioButton*>(target->GetTarget());
      if (targetRadio)
         targetRadio->CopyContentsTo(mValueSelector);
   }
}

void SongBuilder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   for (int i = 0; i < (int)mScenes.size(); ++i)
   {
      if (list == mScenes[i]->mContextMenu)
      {
         switch (mScenes[i]->mContextMenuSelection)
         {
            case ContextMenuItems::kDuplicate:
               DuplicateScene(i);
               break;
            case ContextMenuItems::kDelete:
               if (mScenes.size() > 1)
               {
                  mScenes[i]->CleanUp();
                  mScenes.erase(mScenes.begin() + i);
                  RefreshSequencerDropdowns();
                  i = -1;
               }
               break;
            case ContextMenuItems::kMoveUp:
               if (i > 0)
               {
                  SongScene* scene = mScenes[i];
                  mScenes.erase(mScenes.begin() + i);
                  --i;
                  mScenes.insert(mScenes.begin() + i, scene);
                  RefreshSequencerDropdowns();
               }
               break;
            case ContextMenuItems::kMoveDown:
               if (i < (int)mScenes.size() - 1)
               {
                  SongScene* scene = mScenes[i];
                  mScenes.erase(mScenes.begin() + i);
                  ++i;
                  mScenes.insert(mScenes.begin() + i, scene);
                  RefreshSequencerDropdowns();
               }
               break;
            case ContextMenuItems::kNone:
               break;
         }

         if (i != -1)
            mScenes[i]->mContextMenuSelection = ContextMenuItems::kNone;

         break;
      }
   }

   for (int i = 0; i < kMaxSequencerScenes; ++i)
   {
      if (list == mSequencerContextMenu[i])
      {
         switch (mSequencerContextMenuSelection[i])
         {
            case ContextMenuItems::kDuplicate:
               for (int j = kMaxSequencerScenes - 1; j > i; --j)
               {
                  mSequencerSceneId[j] = mSequencerSceneId[j - 1];
                  mSequencerStepLength[j] = mSequencerStepLength[j - 1];
               }
               break;
            case ContextMenuItems::kDelete:
               for (int j = i; j < kMaxSequencerScenes - 1; ++j)
               {
                  mSequencerSceneId[j] = mSequencerSceneId[j + 1];
                  mSequencerStepLength[j] = mSequencerStepLength[j + 1];
               }
               break;
            case ContextMenuItems::kMoveUp:
               if (i > 0)
               {
                  int temp = mSequencerSceneId[i];
                  mSequencerSceneId[i] = mSequencerSceneId[i - 1];
                  mSequencerSceneId[i - 1] = temp;

                  temp = mSequencerStepLength[i];
                  mSequencerStepLength[i] = mSequencerStepLength[i - 1];
                  mSequencerStepLength[i - 1] = temp;
               }
               break;
            case ContextMenuItems::kMoveDown:
               if (i < kMaxSequencerScenes - 1 && mSequencerSceneId[i + 1] >= 0)
               {
                  int temp = mSequencerSceneId[i];
                  mSequencerSceneId[i] = mSequencerSceneId[i + 1];
                  mSequencerSceneId[i + 1] = temp;

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
   mModuleSaveData.LoadBool("reset_transport_every_sequencer_scene", moduleInfo, true);

   if (IsSpawningOnTheFly(moduleInfo))
   {
      mScenes.push_back(new SongScene("off"));
      mScenes.push_back(new SongScene("intro"));
      mScenes.push_back(new SongScene("verse"));
      mScenes.push_back(new SongScene("prechorus"));
      mScenes.push_back(new SongScene("chorus"));
      mScenes.push_back(new SongScene("bridge"));
      mScenes.push_back(new SongScene("outro"));
      for (auto* scene : mScenes)
         scene->CreateUIControls(this);

      RefreshSequencerDropdowns();
   }

   SetUpFromSaveData();
}

void SongBuilder::SetUpFromSaveData()
{
   mResetOnSceneChange = mModuleSaveData.GetBool("reset_transport_every_sequencer_scene");
}

void SongBuilder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   out << (int)mTargets.size();
   for (auto* target : mTargets)
   {
      out << target->mId;
      target->mCable->SaveState(out);
      out << (int)target->mDisplayType;
   }

   out << (int)mScenes.size();
   for (auto* scene : mScenes)
   {
      out << scene->mName;
      out << scene->mId;
      out << (int)scene->mValues.size();
      for (auto* value : scene->mValues)
      {
         out << value->mId;
         out << value->mFloatValue;
         out << value->mBoolValue;
         out << value->mIntValue;
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
      if (rev >= 1)
         in >> mTargets[i]->mId;
      mTargets[i]->CreateUIControls(this);
      mTargets[i]->mCable->LoadState(in);
      int displayType;
      in >> displayType;
      mTargets[i]->mDisplayType = (ControlTarget::DisplayType)displayType;
   }

   for (auto* scene : mScenes)
      scene->CleanUp();

   int numScenes;
   in >> numScenes;
   mScenes.resize(numScenes);
   for (int i = 0; i < numScenes; ++i)
   {
      mScenes[i] = new SongScene("");
      in >> mScenes[i]->mName;
      in >> mScenes[i]->mId;
      mScenes[i]->CreateUIControls(this);

      int numValues;
      in >> numValues;
      mScenes[i]->mValues.resize(numValues);
      for (int j = 0; j < numValues; ++j)
      {
         mScenes[i]->mValues[j] = new ControlValue();
         in >> mScenes[i]->mValues[j]->mId;
         in >> mScenes[i]->mValues[j]->mFloatValue;
         in >> mScenes[i]->mValues[j]->mBoolValue;
         in >> mScenes[i]->mValues[j]->mIntValue;
         mScenes[i]->mValues[j]->CreateUIControls(this);
         mScenes[i]->TargetControlUpdated(mTargets[j], j, false);
      }
   }

   RefreshSequencerDropdowns();
   mWantRefreshValueDropdowns = true;

   IDrawableModule::LoadState(in, rev);
}

void SongBuilder::DuplicateScene(int sceneIndex)
{
   std::vector<std::string> sceneNames;
   for (auto* scene : mScenes)
      sceneNames.push_back(scene->mName);
   std::string numberless = mScenes[sceneIndex]->mName;
   while (numberless.size() > 1 && isdigit(numberless[numberless.size() - 1]))
      numberless = numberless.substr(0, numberless.size() - 1);
   std::string newSceneName = GetUniqueName(numberless, sceneNames);
   SongScene* newScene = new SongScene(newSceneName);
   newScene->CreateUIControls(this);
   mScenes.insert(mScenes.begin() + sceneIndex + 1, newScene);
   for (auto* value : mScenes[sceneIndex]->mValues)
   {
      newScene->AddValue(this);
      auto* newValue = newScene->mValues[newScene->mValues.size() - 1];
      newValue->mFloatValue = value->mFloatValue;
      newValue->mBoolValue = value->mBoolValue;
      newValue->mIntValue = value->mIntValue;
   }

   RefreshSequencerDropdowns();
}

void SongBuilder::AddTarget()
{
   ControlTarget* target = new ControlTarget();
   target->CreateUIControls(this);
   mTargets.push_back(target);

   for (int i = 0; i < (int)mScenes.size(); ++i)
      mScenes[i]->AddValue(this);
}

void SongBuilder::SongScene::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* scene : owner->mScenes)
      {
         if (scene != this && mId <= scene->mId)
            mId = scene->mId + 1;
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
   mContextMenu->SetCableTargetable(false);
   mContextMenu->SetDisplayStyle(DropdownDisplayStyle::kHamburger);
}

void SongBuilder::SongScene::AddValue(SongBuilder* owner)
{
   ControlValue* value = new ControlValue();
   value->CreateUIControls(owner);
   int index = (int)mValues.size();
   mValues.push_back(value);
   TargetControlUpdated(owner->mTargets[index], index, false);
}

void SongBuilder::SongScene::TargetControlUpdated(SongBuilder::ControlTarget* target, int targetIndex, bool wasManuallyPatched)
{
   IUIControl* control = target->GetTarget();
   if (control != nullptr)
   {
      if (!target->mHadTarget)
      {
         mValues[targetIndex]->mFloatValue = control->GetValue();
         mValues[targetIndex]->mBoolValue = control->GetValue() > 0;
         mValues[targetIndex]->mIntValue = (int)control->GetValue();

         mValues[targetIndex]->UpdateDropdownContents(target);
      }
   }
   else if (wasManuallyPatched) //user intentionally deleted connection
   {
      mValues[targetIndex]->CleanUp();
      mValues.erase(mValues.begin() + targetIndex);
   }
}

void SongBuilder::SongScene::Draw(SongBuilder* owner, float x, float y, int sceneIndex)
{
   float width = GetWidth();
   float height = kRowHeight;
   ofPushStyle();
   ofNoFill();
   ofSetColor(ofColor(150, 150, 150));
   ofRect(x, y, width, height);
   if (sceneIndex == owner->mCurrentScene)
   {
      ofFill();
      ofSetColor(ofColor(130, 130, 130, 130));
      ofRect(x, y, width, height);
   }
   if (sceneIndex == owner->mQueuedScene)
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
      mValues[i]->Draw(x + kSceneTabWidth + i * (kColumnWidth + kSpacingX), y, sceneIndex, owner->mTargets[i]);
}

void SongBuilder::SongScene::MoveValue(int index, int amount)
{
   if (index + amount >= 0 && index + amount < (int)mValues.size())
   {
      ControlValue* value = mValues[index];
      mValues.erase(mValues.begin() + index);
      index += amount;
      mValues.insert(mValues.begin() + index, value);
   }
}

float SongBuilder::SongScene::GetWidth() const
{
   return kSceneTabWidth + (int)mValues.size() * (kColumnWidth + kSpacingX);
}

void SongBuilder::SongScene::CleanUp()
{
   mNameEntry->RemoveFromOwner();
   mActivateButton->RemoveFromOwner();
   mContextMenu->RemoveFromOwner();
   for (auto& value : mValues)
      value->CleanUp();
}

void SongBuilder::ControlTarget::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* target : owner->mTargets)
      {
         if (target != this && target != nullptr && mId <= target->mId)
            mId = target->mId + 1;
      }
   }

   mOwner = owner;

   mCable = new PatchCableSource(owner, kConnectionType_UIControl);
   owner->AddPatchCableSource(mCable);
   mCable->SetAllowMultipleTargets(true);
   mCable->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   mMoveLeftButton = new ClickButton(owner, "move left", -1, -1, ButtonDisplayStyle::kArrowLeft);
   mMoveRightButton = new ClickButton(owner, "move right", -1, -1, ButtonDisplayStyle::kArrowRight);
   mCycleDisplayTypeButton = new ClickButton(owner, "type", -1, -1);
   mColorSelector = new DropdownList(owner, ("color" + ofToString(mId)).c_str(), -1, -1, &mColorIndex, 25);

   // Block modulation cables from connecting to these controls as it behaves wrong (and saves incorrectly) except for the color button, that one is fun and works.
   mMoveLeftButton->SetCableTargetable(false);
   mMoveRightButton->SetCableTargetable(false);
   mCycleDisplayTypeButton->SetCableTargetable(false);

   for (int i = 0; i < (int)owner->mColors.size(); ++i)
      mColorSelector->AddLabel(owner->mColors[i].name, i);
   mColorSelector->SetDrawTriangle(false);
}

void SongBuilder::ControlTarget::Draw(float x, float y, int numRows)
{
   ofPushStyle();
   ofFill();
   ofSetColor(GetColor() * .7f);
   ofRect(x, y, kColumnWidth, kTargetTabHeightTop);
   ofRect(x, y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY), kColumnWidth, kTargetTabHeightBottom);
   ofPopStyle();

   IUIControl* target = GetTarget();
   if (target)
   {
      ofPushMatrix();
      ofClipWindow(x, y, kColumnWidth, kTargetTabHeightTop, true);
      std::string text = target->Path(false, true);
      if (mDisplayType == DisplayType::Checkbox)
         ofStringReplace(text, "~enabled", "");
      std::string displayString;
      const int kSliceSize = 11;
      int cursor = 0;
      for (; cursor + kSliceSize < (int)text.size(); cursor += kSliceSize)
         displayString += text.substr(cursor, kSliceSize) + "\n";
      displayString += text.substr(cursor, (int)text.size() - cursor);
      DrawTextNormal(displayString, x + 2, y + 9, 7);
      ofPopMatrix();
   }
   float bottomY = y + kTargetTabHeightTop + kSpacingY + numRows * (kRowHeight + kSpacingY);
   mCable->SetManualPosition(x + kColumnWidth * .5f, bottomY + 5);
   mCable->SetColor(GetColor());
   mMoveLeftButton->SetPosition(x, bottomY - 3);
   if (gHoveredUIControl == mMoveLeftButton)
      mMoveLeftButton->Draw();
   mMoveRightButton->SetPosition(x + kColumnWidth - 20, bottomY - 3);
   if (gHoveredUIControl == mMoveRightButton)
      mMoveRightButton->Draw();
   mCycleDisplayTypeButton->SetPosition(x, y + kTargetTabHeightTop - 15);
   if (gHoveredUIControl == mCycleDisplayTypeButton)
      mCycleDisplayTypeButton->Draw();
   mColorSelector->SetPosition(x + 25, y + kTargetTabHeightTop - 15);
   if (gHoveredUIControl == mColorSelector)
      mColorSelector->Draw();
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
   if (dynamic_cast<Checkbox*>(target) != nullptr || dynamic_cast<ClickButton*>(target) != nullptr)
      mDisplayType = DisplayType::Checkbox;
   else if (dynamic_cast<DropdownList*>(target) != nullptr || dynamic_cast<RadioButton*>(target) != nullptr)
      mDisplayType = DisplayType::Dropdown;
   else
      mDisplayType = DisplayType::TextEntry;
}

void SongBuilder::ControlTarget::CleanUp()
{
   mCable->GetOwner()->RemovePatchCableSource(mCable);
   mMoveLeftButton->RemoveFromOwner();
   mMoveRightButton->RemoveFromOwner();
   mCycleDisplayTypeButton->RemoveFromOwner();
   mColorSelector->RemoveFromOwner();
}

void SongBuilder::ControlValue::CreateUIControls(SongBuilder* owner)
{
   if (mId == -1)
   {
      //find unique id
      mId = 0;
      for (auto* scene : owner->mScenes)
      {
         for (auto* value : scene->mValues)
         {
            if (value != this && mId <= value->mId)
               mId = value->mId + 1;
         }
      }
   }

   mValueEntry = new TextEntry(owner, ("value " + ofToString(mId)).c_str(), -1, -1, 4, &mFloatValue, -9999, 9999);
   mCheckbox = new Checkbox(owner, ("checkbox " + ofToString(mId)).c_str(), -1, -1, &mBoolValue);
   mCheckbox->SetDisplayText(false);
   mValueSelector = new DropdownList(owner, ("dropdown " + ofToString(mId)).c_str(), -1, -1, &mIntValue, kColumnWidth);
   mValueSelector->SetDrawTriangle(false);

   mValueEntry->SetShowing(false);
   mCheckbox->SetShowing(false);
   mValueSelector->SetShowing(false);
}

void SongBuilder::ControlValue::Draw(float x, float y, int sceneIndex, ControlTarget* target)
{
   ofPushStyle();
   ofFill();
   ofSetColor(target->GetColor() * .7f);
   ofRect(x, y + 2, kColumnWidth, kRowHeight - 4);
   ofPopStyle();

   mValueEntry->SetPosition(x + 7, y + 3);
   mValueEntry->Draw();
   mCheckbox->SetPosition(x + 20, y + 3);
   mCheckbox->Draw();
   mValueSelector->SetPosition(x, y + 2);
   mValueEntry->SetShowing(target->GetTarget() && target->mDisplayType == ControlTarget::DisplayType::TextEntry);
   mCheckbox->SetShowing(target->GetTarget() && target->mDisplayType == ControlTarget::DisplayType::Checkbox);
   mValueSelector->SetShowing(target->GetTarget() && target->mDisplayType == ControlTarget::DisplayType::Dropdown);
   mValueSelector->Draw();
}

void SongBuilder::ControlValue::CleanUp()
{
   mValueEntry->RemoveFromOwner();
   mCheckbox->RemoveFromOwner();
   mValueSelector->RemoveFromOwner();
}