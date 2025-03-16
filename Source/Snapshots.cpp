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
//  Snapshots.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 7/29/13.
//
//

#include "Snapshots.h"
#include "ModularSynth.h"
#include "Slider.h"
#include "ofxJSONElement.h"
#include "PatchCableSource.h"
#include "juce_core/juce_core.h"

std::vector<IUIControl*> Snapshots::sSnapshotHighlightControls;

namespace
{
   const int kListRowHeight = 15;
   const int kMinNumListRows = 4;
   const int kListModeGridWidth = 137;
}

Snapshots::Snapshots()
{
}

Snapshots::~Snapshots()
{
   TheTransport->RemoveAudioPoller(this);
}

void Snapshots::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(this, "uigrid", 5, 38, kListModeGridWidth, kMinNumListRows * kListRowHeight, 1, kMinNumListRows);
   mBlendTimeSlider = new FloatSlider(this, "blend", 5, 20, 87, 15, &mBlendTime, 0, 5000);
   mCurrentSnapshotSelector = new DropdownList(this, "snapshot", 35, 3, &mCurrentSnapshot, 80);
   mRandomizeButton = new ClickButton(this, "random", mBlendTimeSlider, kAnchor_Right);
   mAddButton = new ClickButton(this, "add", mCurrentSnapshotSelector, kAnchor_Right);
   mStoreCheckbox = new Checkbox(this, "store", mAddButton, kAnchor_Right, &mStoreMode);
   mDeleteCheckbox = new Checkbox(this, "delete", mStoreCheckbox, kAnchor_Right, &mDeleteMode);
   mSnapshotLabelEntry = new TextEntry(this, "snapshot label", -1, -1, 12, &mSnapshotLabel);

   {
      mModuleCable = new PatchCableSource(this, kConnectionType_Special);
      ofColor color = IDrawableModule::GetColor(kModuleCategory_Other);
      color.a *= .3f;
      mModuleCable->SetColor(color);
      mModuleCable->SetManualPosition(10, 10);
      mModuleCable->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
      //mModuleCable->SetPatchCableDrawMode(kPatchCableDrawMode_HoverOnly);
      AddPatchCableSource(mModuleCable);
   }

   {
      mUIControlCable = new PatchCableSource(this, kConnectionType_UIControl);
      ofColor color = IDrawableModule::GetColor(kModuleCategory_Modulator);
      color.a *= .3f;
      mUIControlCable->SetColor(color);
      mUIControlCable->SetManualPosition(25, 10);
      mUIControlCable->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
      //mUIControlCable->SetPatchCableDrawMode(kPatchCableDrawMode_HoverOnly);
      AddPatchCableSource(mUIControlCable);
   }

   for (int i = 0; i < 32; ++i)
      mCurrentSnapshotSelector->AddLabel(("snapshot" + ofToString(i)).c_str(), i);
}

void Snapshots::Init()
{
   IDrawableModule::Init();

   int defaultSnapshot = mModuleSaveData.GetInt("defaultsnapshot");
   if (defaultSnapshot != -1)
      SetSnapshot(defaultSnapshot, gTime);

   TheTransport->AddAudioPoller(this);
}

void Snapshots::Poll()
{
   if (mQueuedSnapshotIndex != -1)
   {
      SetSnapshot(mQueuedSnapshotIndex, NextBufferTime(false));
      mQueuedSnapshotIndex = -1;
   }

   if (mDrawSetSnapshotCountdown > 0)
   {
      --mDrawSetSnapshotCountdown;
      if (mDrawSetSnapshotCountdown == 0)
         sSnapshotHighlightControls.clear();
   }

   if (!mBlending && !mBlendRamps.empty())
   {
      mBlendRamps.clear();
   }
}

void Snapshots::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() != mSnapshotLabelEntry)
      mSnapshotRenameIndex = -1;
   mSnapshotLabelEntry->SetShowing(mSnapshotRenameIndex != -1);

   if (mDisplayMode == DisplayMode::Grid)
   {
      ofVec2f pos = mGrid->GetPosition(K(local));
      mSnapshotLabelEntry->SetPosition(pos.x, pos.y);
      mStoreCheckbox->PositionTo(mGrid, kAnchor_Below);
      mDeleteCheckbox->PositionTo(mStoreCheckbox, kAnchor_Right_Padded);
   }

   if (mDisplayMode == DisplayMode::List)
   {
      ofVec2f pos = mGrid->GetPosition(K(local));
      pos.y += mSnapshotRenameIndex * kListRowHeight;
      mSnapshotLabelEntry->SetPosition(pos.x, pos.y);

      mStoreCheckbox->PositionTo(mGrid, kAnchor_Below);
      mDeleteCheckbox->PositionTo(mStoreCheckbox, kAnchor_Right_Padded);
   }

   mGrid->Draw();
   mBlendTimeSlider->Draw();
   mCurrentSnapshotSelector->Draw();
   mRandomizeButton->Draw();
   mAddButton->Draw();
   mStoreCheckbox->Draw();
   mDeleteCheckbox->Draw();

   if (mDisplayMode == DisplayMode::List)
   {
      for (int i = 0; i < (int)mSnapshotCollection.size(); ++i)
      {
         if (!mSnapshotCollection[i].mSnapshots.empty() && mSnapshotRenameIndex != i)
         {
            std::string label = mSnapshotCollection[i].mLabel;
            ofVec2f pos = mGrid->GetCellPosition(i % mGrid->GetCols(), i / mGrid->GetCols()) + mGrid->GetPosition(true);
            DrawTextNormal(label, pos.x + 5, pos.y + 12);
         }
      }
   }

   int hover = mGrid->CurrentHover();
   bool storeMode = (GetKeyModifiers() == kModifier_Shift) || mStoreMode;
   bool deleteMode = (GetKeyModifiers() == kModifier_Alt) || mDeleteMode;
   bool renameMode = (GetKeyModifiers() == kModifier_Command);
   if (storeMode || deleteMode || renameMode)
   {
      if (hover >= 0 && hover < mGrid->GetCols() * mGrid->GetRows())
      {
         ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
         float squareSize = float(mGrid->GetHeight()) / mGrid->GetRows();

         ofPushStyle();
         ofSetColor(0, 0, 0);
         if (storeMode)
         {
            ofFill();
            ofRect(pos.x + squareSize / 2 - 1, pos.y + 3, 2, squareSize - 6, 0);
            ofRect(pos.x + 3, pos.y + squareSize / 2 - 1, squareSize - 6, 2, 0);
         }
         else if (deleteMode && !mSnapshotCollection[hover].mSnapshots.empty())
         {
            ofLine(pos.x + 3, pos.y + 3, pos.x + squareSize - 3, pos.y + squareSize - 3);
            ofLine(pos.x + squareSize - 3, pos.y + 3, pos.x + 3, pos.y + squareSize - 3);
         }
         else if (renameMode)
         {
            ofLine(pos.x + squareSize / 2, pos.y + 4, pos.x + squareSize / 2, pos.y + squareSize - 4);
            ofLine(pos.x + squareSize / 2 - 3, pos.y + 3, pos.x + squareSize / 2 - 1, pos.y + 3);
            ofLine(pos.x + squareSize / 2 + 1, pos.y + 3, pos.x + squareSize / 2 + 3, pos.y + 3);
            ofLine(pos.x + squareSize / 2 - 3, pos.y + squareSize - 3, pos.x + squareSize / 2 - 1, pos.y + squareSize - 3);
            ofLine(pos.x + squareSize / 2 + 1, pos.y + squareSize - 3, pos.x + squareSize / 2 + 3, pos.y + squareSize - 3);
         }
         ofPopStyle();
      }
   }
   else
   {
      if (mCurrentSnapshot < mGrid->GetCols() * mGrid->GetRows())
      {
         ofVec2f pos = mGrid->GetCellPosition(mCurrentSnapshot % mGrid->GetCols(), mCurrentSnapshot / mGrid->GetCols()) + mGrid->GetPosition(true);
         float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
         float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();

         ofPushStyle();
         ofSetColor(255, 255, 255);
         ofSetLineWidth(2);
         ofNoFill();
         ofRect(pos.x, pos.y, xsize, ysize);
         ofPopStyle();
      }
   }

   if (mSnapshotRenameIndex != -1)
   {
      ofRectangle rect = mSnapshotLabelEntry->GetRect(K(local));
      ofPushStyle();
      ofSetColor(ofColor(40, 40, 40));
      ofFill();
      ofRect(rect);
      ofPopStyle();
   }
   mSnapshotLabelEntry->Draw();
}

void Snapshots::DrawModuleUnclipped()
{
   if (mDisplayMode == DisplayMode::Grid)
   {
      int hover = mGrid->CurrentHover();
      if (hover >= 0 && hover < mSnapshotCollection.size())
      {
         std::string tooltip = mSnapshotCollection[hover].mLabel;
         ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
         pos.x += (mGrid->GetWidth() / mGrid->GetCols()) + 3;
         pos.y += (mGrid->GetHeight() / mGrid->GetRows()) / 2;

         float width = GetStringWidth(tooltip);

         ofFill();
         ofSetColor(50, 50, 50);
         ofRect(pos.x, pos.y, width + 10, 15);

         ofNoFill();
         ofSetColor(255, 255, 255);
         ofRect(pos.x, pos.y, width + 10, 15);

         ofSetColor(255, 255, 255);
         DrawTextNormal(tooltip, pos.x + 5, pos.y + 12);
      }
   }
}

bool Snapshots::HasSnapshot(int index) const
{
   return !mSnapshotCollection[index].mSnapshots.empty();
}

void Snapshots::UpdateGridValues()
{
   mGrid->Clear();
   for (int i = 0; i < mGrid->GetRows() * mGrid->GetCols(); ++i)
   {
      float val = 0;
      if (i < mSnapshotCollection.size() &&
          mSnapshotCollection[i].mSnapshots.empty() == false)
         val = .5f;
      mGrid->SetVal(i % mGrid->GetCols(), i / mGrid->GetCols(), val);
   }
}

bool Snapshots::IsTargetingModule(IDrawableModule* module) const
{
   return VectorContains(module, mSnapshotModules);
}

void Snapshots::AddSnapshotTarget(IDrawableModule* target)
{
   if (!IsTargetingModule(target))
   {
      mSnapshotModules.push_back(target);
      mModuleCable->AddPatchCable(target);
   }
}

void Snapshots::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (mGrid->TestClick(x, y, right, true))
   {
      float gridX, gridY;
      mGrid->GetPosition(gridX, gridY, true);
      GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);

      int idx = cell.mCol + cell.mRow * mGrid->GetCols();

      if (GetKeyModifiers() == kModifier_Shift || mStoreMode)
      {
         StoreSnapshot(idx, true);
      }
      else if (GetKeyModifiers() == kModifier_Alt || mDeleteMode)
      {
         DeleteSnapshot(idx);
      }
      else if (GetKeyModifiers() == kModifier_Command)
      {
         mSnapshotRenameIndex = idx;
         if (idx >= 0 && idx < (int)mSnapshotCollection.size())
         {
            mSnapshotLabel = mSnapshotCollection[idx].mLabel;
            mSnapshotLabelEntry->UpdateDisplayString();
         }
         mSnapshotLabelEntry->MakeActiveTextEntry(!K(setCaretToEnd));
         mSnapshotLabelEntry->SelectAll();
      }
      else
      {
         SetSnapshot(idx, NextBufferTime(false));
      }

      UpdateGridValues();
   }
}

bool Snapshots::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   return false;
}

void Snapshots::PlayNote(NoteMessage note)
{
   if (note.velocity > 0 && note.pitch < (int)mSnapshotCollection.size())
   {
      if (mStoreMode)
         StoreSnapshot(note.pitch, true);
      else if (mDeleteMode)
         DeleteSnapshot(note.pitch);
      else
         SetSnapshot(note.pitch, note.time);

      UpdateGridValues();
   }
}

void Snapshots::SetSnapshot(int idx, double time)
{
   if (!mAllowSetOnAudioThread && IsAudioThread())
   {
      mQueuedSnapshotIndex = idx;
      return;
   }

   if (idx < 0 || idx >= (int)mSnapshotCollection.size())
      return;

   if (mAutoStoreOnSwitch && idx != mCurrentSnapshot)
      StoreSnapshot(mCurrentSnapshot, false);

   mCurrentSnapshot = idx;

   if (mBlendTime > 0)
   {
      mRampMutex.lock();
      mBlending = true;
      mBlendProgress = 0;
      mBlendRamps.clear();
   }

   sSnapshotHighlightControls.clear();
   const SnapshotCollection& coll = mSnapshotCollection[idx];
   for (const auto& snapshot : coll.mSnapshots)
   {
      auto context = IClickable::sPathLoadContext;
      IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
      IUIControl* control = TheSynth->FindUIControl(snapshot.mControlPath);
      IClickable::sPathLoadContext = context;

      if (control)
      {
         if (mBlendTime == 0 ||
             snapshot.mHasLFO ||
             !snapshot.mGridContents.empty() ||
             !snapshot.mString.empty())
         {
            control->SetValueDirect(snapshot.mValue, time);

            FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
            if (slider)
            {
               if (snapshot.mHasLFO)
                  slider->AcquireLFO()->Load(snapshot.mLFOSettings);
               else
               {
                  slider->DisableLFO();
                  control->SetValueDirect(snapshot.mValue, time); // Set the value again because a already active LFO can change this.
               }
            }

            UIGrid* grid = dynamic_cast<UIGrid*>(control);
            if (grid)
            {
               for (int col = 0; col < snapshot.mGridCols; ++col)
               {
                  for (int row = 0; row < snapshot.mGridRows; ++row)
                  {
                     grid->SetVal(col, row, snapshot.mGridContents[size_t(col) + size_t(row) * snapshot.mGridCols]);
                  }
               }
            }

            TextEntry* textEntry = dynamic_cast<TextEntry*>(control);
            if (textEntry && textEntry->GetTextEntryType() == kTextEntry_Text)
               textEntry->SetText(snapshot.mString);

            if (control->ShouldSerializeForSnapshot() && !snapshot.mString.empty())
            {
               juce::MemoryBlock outputStream;
               outputStream.fromBase64Encoding(snapshot.mString);
               FileStreamIn in(outputStream);
               control->LoadState(in, true);
            }
         }
         else
         {
            ControlRamp ramp;
            ramp.mUIControl = control;
            ramp.mRamp.Start(0, control->GetValue(), snapshot.mValue, mBlendTime);
            mBlendRamps.push_back(ramp);
         }

         sSnapshotHighlightControls.push_back(control);
      }
   }

   for (const auto& moduleData : coll.mModuleData)
   {
      IDrawableModule* module = TheSynth->FindModule(moduleData.mModulePath);
      if (module != nullptr && module->ShouldSerializeForSnapshot() && !moduleData.mData.empty())
      {
         juce::MemoryBlock outputStream;
         outputStream.fromBase64Encoding(moduleData.mData);
         FileStreamIn in(outputStream);
         int dataRev;
         in >> dataRev;
         module->LoadState(in, dataRev);
      }
   }

   if (mBlendTime > 0)
   {
      mRampMutex.unlock();
   }

   mDrawSetSnapshotCountdown = 30;
   mSnapshotLabel = coll.mLabel;
}

void Snapshots::RandomizeTargets()
{
   for (int i = 0; i < mSnapshotControls.size(); ++i)
      RandomizeControl(mSnapshotControls[i]);
   for (int i = 0; i < mSnapshotModules.size(); ++i)
   {
      for (auto* control : mSnapshotModules[i]->GetUIControls())
         RandomizeControl(control);
   }
}

void Snapshots::RandomizeControl(IUIControl* control)
{
   if (strcmp(control->Name(), "enabled") == 0) //don't randomize enabled/disable checkbox, too annoying
      return;
   if (dynamic_cast<ClickButton*>(control) != nullptr)
      return;
   control->SetFromMidiCC(ofRandom(1), NextBufferTime(false), true);
}

void Snapshots::OnTransportAdvanced(float amount)
{
   if (mBlending)
   {
      mRampMutex.lock();

      mBlendProgress += amount * TheTransport->MsPerBar();

      for (auto& ramp : mBlendRamps)
      {
         ramp.mUIControl->SetValueDirect(ramp.mRamp.Value(mBlendProgress), gTime);
      }

      if (mBlendProgress >= mBlendTime)
         mBlending = false;

      mRampMutex.unlock();
   }
}

void Snapshots::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (TheSynth->IsLoadingState())
      return;

   auto numModules = mSnapshotModules.size();
   auto numControls = mSnapshotControls.size();

   mSnapshotModules.clear();
   for (auto cable : mModuleCable->GetPatchCables())
      mSnapshotModules.push_back(static_cast<IDrawableModule*>(cable->GetTarget()));
   mSnapshotControls.clear();
   for (auto cable : mUIControlCable->GetPatchCables())
      mSnapshotControls.push_back(static_cast<IUIControl*>(cable->GetTarget()));

   if (mSnapshotModules.size() < numModules || mSnapshotControls.size() < numControls) //we removed something, clean up any snapshots that refer to it
   {
      for (auto& square : mSnapshotCollection)
      {
         std::vector<Snapshot> toRemove;
         for (const auto& snapshot : square.mSnapshots)
         {
            if (!IsConnectedToPath(snapshot.mControlPath))
               toRemove.push_back(snapshot);
         }

         for (auto remove : toRemove)
            square.mSnapshots.remove(remove);
      }
   }
}

bool Snapshots::IsConnectedToPath(std::string path) const
{
   auto context = IClickable::sPathLoadContext;
   IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
   IUIControl* control = TheSynth->FindUIControl(path);
   IClickable::sPathLoadContext = context;

   if (control == nullptr)
      return false;

   if (VectorContains(control, mSnapshotControls))
      return true;

   if (VectorContains(control->GetModuleParent(), mSnapshotModules))
      return true;

   return false;
}

void Snapshots::StoreSnapshot(int idx, bool setAsCurrent)
{
   assert(idx >= 0 && idx < mSnapshotCollection.size());

   SnapshotCollection& coll = mSnapshotCollection[idx];
   coll.mSnapshots.clear();
   coll.mModuleData.clear();

   for (int i = 0; i < mSnapshotControls.size(); ++i)
   {
      coll.mSnapshots.push_back(Snapshot(mSnapshotControls[i], this));
   }
   for (int i = 0; i < mSnapshotModules.size(); ++i)
   {
      for (auto* control : mSnapshotModules[i]->GetUIControls())
      {
         if (dynamic_cast<ClickButton*>(control) == nullptr)
            coll.mSnapshots.push_back(Snapshot(control, this));
      }
      for (auto* grid : mSnapshotModules[i]->GetUIGrids())
         coll.mSnapshots.push_back(Snapshot(grid, this));
      if (mSnapshotModules[i]->ShouldSerializeForSnapshot())
         coll.mModuleData.push_back(SnapshotModuleData(mSnapshotModules[i]));
   }

   mSnapshotLabel = coll.mLabel;

   if (setAsCurrent)
      mCurrentSnapshot = idx;

   UpdateGridValues();
}

void Snapshots::DeleteSnapshot(int idx)
{
   if (idx >= 0 && idx < mSnapshotCollection.size())
   {
      SnapshotCollection& coll = mSnapshotCollection[idx];
      coll.mSnapshots.clear();
      coll.mLabel = "snapshot" + ofToString(idx);
      mCurrentSnapshotSelector->SetLabel(coll.mLabel, idx);
   }
}

bool Snapshots::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;
         int index = x + (y - 1) * 8;

         if (x == 0 && y == 0)
         {
            mStoreMode = midiValue > 0;
         }
         else if (x == 1 && y == 0)
         {
            mDeleteMode = midiValue > 0;
         }
         else if (midiValue > 0 && index >= 0 && index < (int)mSnapshotCollection.size())
         {
            if (mStoreMode)
               StoreSnapshot(index, true);
            else if (mDeleteMode)
               DeleteSnapshot(index);
            else
               SetSnapshot(index, gTime);

            UpdateGridValues();
         }

         return true;
      }
   }

   return false;
}

void Snapshots::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor;
         int index = x + (y - 1) * 8;

         if (x == 0 && y == 0)
         {
            if (mStoreMode)
               pushColor = 126;
            else
               pushColor = 86;
         }
         else if (x == 1 && y == 0)
         {
            if (mDeleteMode)
               pushColor = 127;
            else
               pushColor = 114;
         }
         else if (index >= 0 && index < (int)mSnapshotCollection.size())
         {
            if (index == mCurrentSnapshot)
               pushColor = 120;
            else if (mSnapshotCollection[index].mSnapshots.empty() == false)
               pushColor = 125;
            else
               pushColor = 20;
         }
         else
         {
            pushColor = 0;
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

void Snapshots::ResizeSnapshotCollection(int size)
{
   int oldSize = (int)mSnapshotCollection.size();
   if (size != oldSize)
   {
      mSnapshotCollection.resize(size);
      for (int i = oldSize; i < size; ++i)
         mSnapshotCollection[i].mLabel = "snapshot" + ofToString(i);

      if (mDisplayMode == DisplayMode::List)
         mModuleSaveData.SetInt("num_list_snapshots", size);
   }
}

namespace
{
   const float extraW = 9;
   const float extraH = 58;
   const float gridSquareDimension = 18;
   const int maxGridSide = 20;
}

void Snapshots::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizeButton)
      RandomizeTargets();

   if (button == mAddButton)
   {
      for (int i = 0; i < (int)mSnapshotCollection.size() + 1; ++i)
      {
         if (i == mSnapshotCollection.size()) //we didn't find any
            ResizeSnapshotCollection(i + 1);
         if (mSnapshotCollection[i].mSnapshots.empty())
         {
            StoreSnapshot(i, true);
            mCurrentSnapshot = i;
            if (mDisplayMode == DisplayMode::List)
               UpdateListGrid();
            if (mDisplayMode == DisplayMode::Grid)
               UpdateGridValues();
            break;
         }
      }
   }
}

void Snapshots::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mCurrentSnapshotSelector)
   {
      int newIdx = mCurrentSnapshot;
      mCurrentSnapshot = oldVal;
      if (mStoreMode)
         StoreSnapshot(newIdx, true);
      else if (mDeleteMode)
         DeleteSnapshot(newIdx);
      else
         SetSnapshot(newIdx, time);
      UpdateGridValues();
   }
}

void Snapshots::TextEntryComplete(TextEntry* entry)
{
   if (entry == mSnapshotLabelEntry)
   {
      int snapshotIndex = mCurrentSnapshot;
      if (mSnapshotRenameIndex != -1)
      {
         snapshotIndex = mSnapshotRenameIndex;
         mSnapshotRenameIndex = -1;
      }

      mSnapshotCollection[snapshotIndex].mLabel = mSnapshotLabel;
      mCurrentSnapshotSelector->SetLabel(mSnapshotLabel, snapshotIndex);
   }
}

void Snapshots::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void Snapshots::Resize(float w, float h)
{
   SetGridSize(MAX(w - extraW, 137), MAX(h - extraH, gridSquareDimension));
}

void Snapshots::SetGridSize(float w, float h)
{
   assert(mDisplayMode == DisplayMode::Grid);
   mGrid->SetDimensions(w, h);
   int cols = MIN(w / gridSquareDimension, maxGridSide);
   int rows = MIN(h / gridSquareDimension, maxGridSide);
   mGrid->SetGrid(cols, rows);
   ResizeSnapshotCollection(cols * rows);
   UpdateGridValues();
}

void Snapshots::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("defaultsnapshot", moduleInfo, -1, -1, 100, true);
   EnumMap map;
   map["grid"] = (int)DisplayMode::Grid;
   map["list"] = (int)DisplayMode::List;
   mModuleSaveData.LoadEnum<DisplayMode>("display_mode", moduleInfo, (int)DisplayMode::List, nullptr, &map);
   mModuleSaveData.LoadInt("num_list_snapshots", moduleInfo, 8, 0, 64, K(isTextField));
   mModuleSaveData.LoadBool("allow_set_on_audio_thread", moduleInfo, true);
   mModuleSaveData.LoadBool("auto_store_on_switch", moduleInfo, false);

   //for rev < 4
   mOldWidth = moduleInfo["gridwidth"].asFloat();
   mOldHeight = moduleInfo["gridheight"].asFloat();

   SetUpFromSaveData();
}

void Snapshots::SetUpFromSaveData()
{
   mAllowSetOnAudioThread = mModuleSaveData.GetBool("allow_set_on_audio_thread");
   mDisplayMode = mModuleSaveData.GetEnum<DisplayMode>("display_mode");
   mAutoStoreOnSwitch = mModuleSaveData.GetBool("auto_store_on_switch");

   if (mDisplayMode == DisplayMode::Grid)
   {
      // set up the grid
      float w, h;
      GetModuleDimensions(w, h);
      Resize(w, h);
   }

   if (mDisplayMode == DisplayMode::List)
   {
      ResizeSnapshotCollection(mModuleSaveData.GetInt("num_list_snapshots"));
      UpdateListGrid();
   }
}

void Snapshots::UpdateListGrid()
{
   int numSnapshots = (int)mSnapshotCollection.size();
   mGrid->SetGrid(1, numSnapshots);
   mGrid->SetDimensions(kListModeGridWidth, kListRowHeight * numSnapshots);
   UpdateGridValues();
}

void Snapshots::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mSnapshotCollection.size();
   for (auto& coll : mSnapshotCollection)
   {
      out << (int)coll.mSnapshots.size();
      for (auto& snapshot : coll.mSnapshots)
      {
         out << snapshot.mControlPath;
         out << snapshot.mValue;
         out << snapshot.mHasLFO;
         snapshot.mLFOSettings.SaveState(out);
         out << snapshot.mGridCols;
         out << snapshot.mGridRows;
         assert(snapshot.mGridContents.size() == size_t(snapshot.mGridCols) * snapshot.mGridRows);
         for (size_t i = 0; i < snapshot.mGridContents.size(); ++i)
            out << snapshot.mGridContents[i];
         out << snapshot.mString;
      }
      out << coll.mLabel;
   }

   out << (int)mSnapshotModules.size();
   for (auto module : mSnapshotModules)
   {
      if (module != nullptr && !module->IsDeleted())
         out << module->Path();
      else
         out << std::string("");
   }

   out << (int)mSnapshotControls.size();
   for (auto control : mSnapshotControls)
   {
      if (control != nullptr && !control->GetModuleParent()->IsDeleted())
         out << control->Path();
      else
         out << std::string("");
   }

   out << mCurrentSnapshot;

   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
}

void Snapshots::LoadState(FileStreamIn& in, int rev)
{
   mLoadRev = rev;

   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int collSize;
   in >> collSize;
   mSnapshotCollection.resize(collSize);
   for (int i = 0; i < collSize; ++i)
   {
      int snapshotSize;
      in >> snapshotSize;
      mSnapshotCollection[i].mSnapshots.resize(snapshotSize);
      for (auto& snapshotData : mSnapshotCollection[i].mSnapshots)
      {
         in >> snapshotData.mControlPath;
         in >> snapshotData.mValue;
         in >> snapshotData.mHasLFO;
         snapshotData.mLFOSettings.LoadState(in);
         in >> snapshotData.mGridCols;
         in >> snapshotData.mGridRows;
         if (rev < 3)
         {
            // Check if the loaded values are within an acceptable range.
            // This is done because mGridCols and mGridRows could previously be saved with random values since they were not properly initialized.
            if (snapshotData.mGridCols < 0 || snapshotData.mGridCols > 1000)
               snapshotData.mGridCols = 0;
            if (snapshotData.mGridRows < 0 || snapshotData.mGridRows > 1000)
               snapshotData.mGridRows = 0;
         }
         snapshotData.mGridContents.resize(size_t(snapshotData.mGridCols) * snapshotData.mGridRows);
         for (int k = 0; k < snapshotData.mGridCols * snapshotData.mGridRows; ++k)
            in >> snapshotData.mGridContents[k];
         in >> snapshotData.mString;
      }
      in >> mSnapshotCollection[i].mLabel;
      if (rev < 2 && mSnapshotCollection[i].mLabel.empty())
         mSnapshotCollection[i].mLabel = "snapshot" + ofToString(i);
      mCurrentSnapshotSelector->SetLabel(mSnapshotCollection[i].mLabel, i);
   }

   UpdateGridValues();

   std::string path;
   int size;
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      in >> path;
      if (path != "")
      {
         IDrawableModule* module = TheSynth->FindModule(path);
         if (module)
         {
            mSnapshotModules.push_back(module);
            mModuleCable->AddPatchCable(module);
         }
      }
   }
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      in >> path;
      if (path != "")
      {
         IUIControl* control = TheSynth->FindUIControl(path);
         if (control)
         {
            mSnapshotControls.push_back(control);
            mUIControlCable->AddPatchCable(control);
         }
      }
   }

   if (rev >= 2)
      in >> mCurrentSnapshot;

   if (rev < 4)
   {
      mDisplayMode = DisplayMode::Grid;
      mModuleSaveData.SetEnum("display_mode", DisplayMode::Grid);
   }

   if (rev >= 4)
   {
      float w, h;
      in >> w;
      in >> h;
      if (mDisplayMode == DisplayMode::Grid)
         SetGridSize(w, h);
   }
   else
   {
      SetGridSize(mOldWidth, mOldHeight);
   }
}

void Snapshots::UpdateOldControlName(std::string& oldName)
{
   IDrawableModule::UpdateOldControlName(oldName);

   if (oldName == "blend ms")
      oldName = "blend";
   if (oldName == "preset")
      oldName = "snapshot";
}

bool Snapshots::LoadOldControl(FileStreamIn& in, std::string& oldName)
{
   if (mLoadRev < 2)
   {
      if (oldName == "preset")
      {
         //load from int slider
         int intSliderRev;
         in >> intSliderRev;
         in >> mCurrentSnapshot;
         int dummy;
         if (intSliderRev >= 1)
         {
            in >> dummy;
            in >> dummy;
         }
         return true;
      }
   }
   if (mLoadRev < 4)
   {
      if (oldName == "auto-store on switch")
      {
         //load from checkbox
         int checkboxSliderRev;
         in >> checkboxSliderRev;
         float var;
         in >> var;
         mAutoStoreOnSwitch = var > 0.5f;
         mModuleSaveData.SetBool("auto_store_on_switch", mAutoStoreOnSwitch);
         return true;
      }
   }
   return false;
}

std::vector<IUIControl*> Snapshots::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mCurrentSnapshotSelector);
   ignore.push_back(mSnapshotLabelEntry);
   return ignore;
}

Snapshots::Snapshot::Snapshot(IUIControl* control, Snapshots* snapshots)
{
   auto context = IClickable::sPathSaveContext;
   IClickable::sPathSaveContext = snapshots->GetParent() ? snapshots->GetParent()->Path() + "~" : "";
   mControlPath = control->Path();
   IClickable::sPathSaveContext = context;

   mValue = control->GetValue();

   FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
   if (slider)
   {
      FloatSliderLFOControl* lfo = slider->GetLFO();
      if (lfo && lfo->Active())
      {
         mHasLFO = true;
         mLFOSettings = lfo->GetSettings();
      }
      else
      {
         mHasLFO = false;
      }
   }
   else
   {
      mHasLFO = false;
   }

   UIGrid* grid = dynamic_cast<UIGrid*>(control);
   if (grid)
   {
      mGridCols = grid->GetCols();
      mGridRows = grid->GetRows();
      mGridContents.resize(size_t(grid->GetCols()) * grid->GetRows());
      for (int col = 0; col < grid->GetCols(); ++col)
      {
         for (int row = 0; row < grid->GetRows(); ++row)
         {
            mGridContents[size_t(col) + size_t(row) * grid->GetCols()] = grid->GetVal(col, row);
         }
      }
   }

   TextEntry* textEntry = dynamic_cast<TextEntry*>(control);
   if (textEntry && textEntry->GetTextEntryType() == kTextEntry_Text)
      mString = textEntry->GetText();

   if (control->ShouldSerializeForSnapshot())
   {
      juce::MemoryBlock tempBlock;
      {
         FileStreamOut out(tempBlock);
         control->SaveState(out);
      }
      mString = tempBlock.toBase64Encoding().toStdString();
   }
}

Snapshots::SnapshotModuleData::SnapshotModuleData(IDrawableModule* module)
{
   mModulePath = module->Path();
   juce::MemoryBlock tempBlock;
   {
      FileStreamOut out(tempBlock);
      module->SaveState(out);
   }
   mData = tempBlock.toBase64Encoding().toStdString();
}
