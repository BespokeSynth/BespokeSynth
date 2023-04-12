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

std::vector<IUIControl*> Snapshots::sSnapshotHighlightControls;

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
   mGrid = new UIGrid("uigrid", 5, 38, 120, 50, 8, 3, this);
   mBlendTimeSlider = new FloatSlider(this, "blend", 5, 20, 70, 15, &mBlendTime, 0, 5000);
   mCurrentSnapshotSelector = new DropdownList(this, "snapshot", 35, 3, &mCurrentSnapshot, 64);
   mRandomizeButton = new ClickButton(this, "random", 78, 20);
   mAddButton = new ClickButton(this, "add", 101, 3);
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
      mCurrentSnapshotSelector->AddLabel(ofToString(i).c_str(), i);
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

   mGrid->Draw();
   mBlendTimeSlider->Draw();
   mCurrentSnapshotSelector->Draw();
   mRandomizeButton->Draw();
   mAddButton->Draw();
   mSnapshotLabelEntry->SetPosition(3, mGrid->GetRect(K(local)).getMaxY() + 3);
   mSnapshotLabelEntry->Draw();

   int hover = mGrid->CurrentHover();
   bool shiftHeld = GetKeyModifiers() == kModifier_Shift;
   bool altHeld = GetKeyModifiers() == kModifier_Alt;
   if (shiftHeld || altHeld)
   {
      if (hover < mGrid->GetCols() * mGrid->GetRows())
      {
         ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
         float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
         float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();

         ofPushStyle();
         ofSetColor(0, 0, 0);
         if (shiftHeld)
         {
            ofFill();
            ofRect(pos.x + xsize / 2 - 1, pos.y + 3, 2, ysize - 6, 0);
            ofRect(pos.x + 3, pos.y + ysize / 2 - 1, xsize - 6, 2, 0);
         }
         if (altHeld && !mSnapshotCollection[hover].mSnapshots.empty())
         {
            ofLine(pos.x + 3, pos.y + 3, pos.x + xsize - 3, pos.y + ysize - 3);
            ofLine(pos.x + xsize - 3, pos.y + 3, pos.x + 3, pos.y + ysize - 3);
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
}

void Snapshots::DrawModuleUnclipped()
{
   int hover = mGrid->CurrentHover();
   if (hover != -1 && !mSnapshotCollection.empty())
   {
      assert(hover >= 0 && hover < mSnapshotCollection.size());

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

void Snapshots::UpdateGridValues()
{
   mGrid->Clear();
   assert(mSnapshotCollection.size() >= size_t(mGrid->GetRows()) * mGrid->GetCols());
   for (int i = 0; i < mGrid->GetRows() * mGrid->GetCols(); ++i)
   {
      float val = 0;
      if (mSnapshotCollection[i].mSnapshots.empty() == false)
         val = .5f;
      mGrid->SetVal(i % mGrid->GetCols(), i / mGrid->GetCols(), val);
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

      if (GetKeyModifiers() == kModifier_Shift)
         Store(idx);
      else if (GetKeyModifiers() == kModifier_Alt)
         Delete(idx);
      else
         SetSnapshot(idx, NextBufferTime(false));

      UpdateGridValues();
   }
}

bool Snapshots::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   return false;
}

void Snapshots::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0 && pitch < (int)mSnapshotCollection.size())
   {
      SetSnapshot(pitch, time);
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

   if (mAutoStoreOnSwitch)
      Store(mCurrentSnapshot);

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
   for (std::list<Snapshot>::const_iterator i = coll.mSnapshots.begin();
        i != coll.mSnapshots.end(); ++i)
   {
      auto context = IClickable::sPathLoadContext;
      IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
      IUIControl* control = TheSynth->FindUIControl(i->mControlPath);
      IClickable::sPathLoadContext = context;

      if (control)
      {
         if (mBlendTime == 0 ||
             i->mHasLFO ||
             !i->mGridContents.empty() ||
             !i->mString.empty())
         {
            control->SetValueDirect(i->mValue, time);

            FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
            if (slider)
            {
               if (i->mHasLFO)
                  slider->AcquireLFO()->Load(i->mLFOSettings);
               else
                  slider->DisableLFO();
            }

            UIGrid* grid = dynamic_cast<UIGrid*>(control);
            if (grid)
            {
               for (int col = 0; col < i->mGridCols; ++col)
               {
                  for (int row = 0; row < i->mGridRows; ++row)
                  {
                     grid->SetVal(col, row, i->mGridContents[size_t(col) + size_t(row) * i->mGridCols]);
                  }
               }
            }

            TextEntry* textEntry = dynamic_cast<TextEntry*>(control);
            if (textEntry && textEntry->GetTextEntryType() == kTextEntry_Text)
               textEntry->SetText(i->mString);
         }
         else
         {
            ControlRamp ramp;
            ramp.mUIControl = control;
            ramp.mRamp.Start(0, control->GetValue(), i->mValue, mBlendTime);
            mBlendRamps.push_back(ramp);
         }

         sSnapshotHighlightControls.push_back(control);
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

void Snapshots::Store(int idx)
{
   assert(idx >= 0 && idx < mSnapshotCollection.size());

   SnapshotCollection& coll = mSnapshotCollection[idx];
   coll.mSnapshots.clear();

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
   }

   mSnapshotLabel = coll.mLabel;
}

void Snapshots::Delete(int idx)
{
   assert(idx >= 0 && idx < mSnapshotCollection.size());

   SnapshotCollection& coll = mSnapshotCollection[idx];
   coll.mSnapshots.clear();
   coll.mLabel = ofToString(idx);
   mCurrentSnapshotSelector->SetLabel(coll.mLabel, idx);
}

namespace
{
   const float extraW = 10;
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
      for (size_t i = 0; i < mSnapshotCollection.size(); ++i)
      {
         if (mSnapshotCollection[i].mSnapshots.empty())
         {
            Store(i);
            mCurrentSnapshot = i;
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
      SetSnapshot(newIdx, time);
      UpdateGridValues();
   }
}

void Snapshots::TextEntryComplete(TextEntry* entry)
{
   if (entry == mSnapshotLabelEntry)
   {
      mSnapshotCollection[mCurrentSnapshot].mLabel = mSnapshotLabel;
      mCurrentSnapshotSelector->SetLabel(mSnapshotLabel, mCurrentSnapshot);
   }
}

void Snapshots::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void Snapshots::Resize(float w, float h)
{
   SetGridSize(MAX(w - extraW, 120), MAX(h - extraH, gridSquareDimension));
}

void Snapshots::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
   int cols = MIN(w / gridSquareDimension, maxGridSide);
   int rows = MIN(h / gridSquareDimension, maxGridSide);
   mGrid->SetGrid(cols, rows);
   int oldSize = (int)mSnapshotCollection.size();
   if (oldSize < size_t(cols) * rows)
   {
      mSnapshotCollection.resize(size_t(cols) * rows);
      for (int i = oldSize; i < (int)mSnapshotCollection.size(); ++i)
         mSnapshotCollection[i].mLabel = ofToString(i);
   }
   UpdateGridValues();
}

void Snapshots::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["gridwidth"] = mGrid->GetWidth();
   moduleInfo["gridheight"] = mGrid->GetHeight();
}

void Snapshots::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("defaultsnapshot", moduleInfo, -1, -1, 100, true);

   mModuleSaveData.LoadFloat("gridwidth", moduleInfo, 120, 120, 1000);
   mModuleSaveData.LoadFloat("gridheight", moduleInfo, 50, 15, 1000);
   mModuleSaveData.LoadBool("allow_set_on_audio_thread", moduleInfo, true);
   mModuleSaveData.LoadBool("auto_store_on_switch", moduleInfo, false);

   SetUpFromSaveData();
}

void Snapshots::SetUpFromSaveData()
{
   SetGridSize(mModuleSaveData.GetFloat("gridwidth"), mModuleSaveData.GetFloat("gridheight"));
   mAllowSetOnAudioThread = mModuleSaveData.GetBool("allow_set_on_audio_thread");
   mAutoStoreOnSwitch = mModuleSaveData.GetBool("auto_store_on_switch");
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
      out << module->Path();

   out << (int)mSnapshotControls.size();
   for (auto control : mSnapshotControls)
      out << control->Path();

   out << mCurrentSnapshot;
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
      int j = 0;
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
         mSnapshotCollection[i].mLabel = ofToString(i);
      mCurrentSnapshotSelector->SetLabel(mSnapshotCollection[i].mLabel, i);
   }

   UpdateGridValues();

   std::string path;
   int size;
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      in >> path;
      IDrawableModule* module = TheSynth->FindModule(path);
      if (module)
      {
         mSnapshotModules.push_back(module);
         mModuleCable->AddPatchCable(module);
      }
   }
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      in >> path;
      IUIControl* control = TheSynth->FindUIControl(path);
      if (control)
      {
         mSnapshotControls.push_back(control);
         mUIControlCable->AddPatchCable(control);
      }
   }

   if (rev >= 2)
      in >> mCurrentSnapshot;
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
   return false;
}

std::vector<IUIControl*> Snapshots::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mCurrentSnapshotSelector);
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
}
