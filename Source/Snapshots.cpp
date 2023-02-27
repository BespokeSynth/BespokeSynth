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

#include <utility>
#include "ModularSynth.h"
#include "Slider.h"
#include "ofxJSONElement.h"
#include "PatchCableSource.h"

std::vector<IUIControl*> Snapshots::sSnapshotHighlightControls;

Snapshots::Snapshots() = default;

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
      mCurrentSnapshotSelector->AddLabel(ofToString(i), i);
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

   const int hover = mGrid->CurrentHover();
   const bool shiftHeld = GetKeyModifiers() == kModifier_Shift;
   if (shiftHeld)
   {
      if (hover < mGrid->GetCols() * mGrid->GetRows())
      {
         const ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
         const float xsize = mGrid->GetWidth() / mGrid->GetCols();
         const float ysize = mGrid->GetHeight() / mGrid->GetRows();

         ofPushStyle();
         ofSetColor(0, 0, 0);
         ofFill();
         ofRect(pos.x + xsize / 2 - 1, pos.y + 3, 2, ysize - 6, 0);
         ofRect(pos.x + 3, pos.y + ysize / 2 - 1, xsize - 6, 2, 0);
         ofPopStyle();
      }
   }

   if (!shiftHeld)
   {
      if (mCurrentSnapshot < mGrid->GetCols() * mGrid->GetRows())
      {
         const ofVec2f pos = mGrid->GetCellPosition(mCurrentSnapshot % mGrid->GetCols(), mCurrentSnapshot / mGrid->GetCols()) + mGrid->GetPosition(true);
         const float xsize = mGrid->GetWidth() / mGrid->GetCols();
         const float ysize = mGrid->GetHeight() / mGrid->GetRows();

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
   const int hover = mGrid->CurrentHover();
   if (hover != -1 && !mSnapshotCollection.empty())
   {
      assert(hover >= 0 && hover < mSnapshotCollection.size());

      const std::string tooltip = mSnapshotCollection[hover].mLabel;
      ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
      pos.x += (mGrid->GetWidth() / mGrid->GetCols()) + 3;
      pos.y += (mGrid->GetHeight() / mGrid->GetRows()) / 2;

      const float width = GetStringWidth(tooltip);

      ofFill();
      ofSetColor(50, 50, 50);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofSetColor(255, 255, 255);
      DrawTextNormal(tooltip, pos.x + 5, pos.y + 12);
   }
#ifdef DEBUG
   mDrawDebug = true;
#endif
   if (mDrawDebug)
   {
      float w, h;
      this->GetDimensions(w, h);
      w += 8;
      float y = 0;
      DrawTextNormal("Collection: " + ofToString(mSnapshotCollection.size()), w, y += 15);
      DrawTextNormal("Controls: " + ofToString(mSnapshotControls.size()), w, y += 15);
      DrawTextNormal("Modules: " + ofToString(mSnapshotModules.size()), w, y += 15);
   }
}

void Snapshots::UpdateGridValues()
{
   mGrid->Clear();
   assert(mSnapshotCollection.size() >= static_cast<size_t>(mGrid->GetRows()) * mGrid->GetCols());
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
      const GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);

      mCurrentSnapshot = cell.mCol + cell.mRow * mGrid->GetCols();

      if (GetKeyModifiers() == kModifier_Shift)
         Store(mCurrentSnapshot);
      else
         SetSnapshot(mCurrentSnapshot, NextBufferTime(false));

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
   if (velocity > 0 && pitch < static_cast<int>(mSnapshotCollection.size()))
   {
      mCurrentSnapshot = pitch;
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

   if (idx < 0 || idx >= static_cast<int>(mSnapshotCollection.size()))
      return;

   if (mBlendTime > 0)
   {
      mRampMutex.lock();
      mBlending = true;
      mBlendProgress = 0;
      mBlendRamps.clear();
   }

   sSnapshotHighlightControls.clear();
   const SnapshotCollection& coll = mSnapshotCollection[idx];
   for (auto const& i : coll.mSnapshots)
   {
      const auto context = IClickable::sPathLoadContext;
      IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
      IUIControl* control = TheSynth->FindUIControl(i.mControlPath);
      IClickable::sPathLoadContext = context;

      if (control)
      {
         if (mBlendTime == 0 ||
             i.mHasLFO ||
             !i.mGridContents.empty() ||
             !i.mString.empty())
         {
            control->SetValueDirect(i.mValue, time);

            const auto slider = dynamic_cast<FloatSlider*>(control);
            if (slider)
            {
               if (i.mHasLFO)
                  slider->AcquireLFO()->Load(i.mLFOSettings);
               else
                  slider->DisableLFO();
            }

            const auto grid = dynamic_cast<UIGrid*>(control);
            if (grid)
            {
               for (int col = 0; col < i.mGridCols; ++col)
               {
                  for (int row = 0; row < i.mGridRows; ++row)
                  {
                     grid->SetVal(col, row, i.mGridContents[static_cast<size_t>(col) + static_cast<size_t>(row) * i.mGridCols]);
                  }
               }
            }

            const auto textEntry = dynamic_cast<TextEntry*>(control);
            if (textEntry && textEntry->GetTextEntryType() == kTextEntry_Text)
               textEntry->SetText(i.mString);
         }
         else
         {
            ControlRamp ramp;
            ramp.mUIControl = control;
            ramp.mRamp.Start(0, control->GetValue(), i.mValue, mBlendTime);
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
   for (const auto& mSnapshotControl : mSnapshotControls)
      RandomizeControl(mSnapshotControl);
   for (const auto& mSnapshotModule : mSnapshotModules)
   {
      for (auto* control : mSnapshotModule->GetUIControls())
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

void Snapshots::OnTransportAdvanced(const float amount)
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

   const auto numModules = mSnapshotModules.size();
   const auto numControls = mSnapshotControls.size();

   mSnapshotModules.clear();
   for (const auto cable : mModuleCable->GetPatchCables())
   {
      auto target = dynamic_cast<IDrawableModule*>(cable->GetTarget());
      if (target)
         mSnapshotModules.push_back(target);
   }
   mSnapshotControls.clear();
   for (const auto cable : mUIControlCable->GetPatchCables())
   {
      auto target = dynamic_cast<IUIControl*>(cable->GetTarget());
      if (target)
         mSnapshotControls.push_back(target);
   }

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

         for (auto& remove : toRemove)
            square.mSnapshots.remove(remove);
      }
   }
}

bool Snapshots::IsConnectedToPath(const std::string& path) const
{
   const auto context = IClickable::sPathLoadContext;
   IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
   const auto control = TheSynth->FindUIControl(path);
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

   for (const auto& mSnapshotControl : mSnapshotControls)
   {
      coll.mSnapshots.emplace_back(mSnapshotControl, this);
   }
   for (const auto& mSnapshotModule : mSnapshotModules)
   {
      for (auto* control : mSnapshotModule->GetUIControls())
      {
         if (dynamic_cast<ClickButton*>(control) == nullptr)
            coll.mSnapshots.emplace_back(control, this);
      }
      for (auto* grid : mSnapshotModule->GetUIGrids())
         coll.mSnapshots.emplace_back(grid, this);
   }

   mSnapshotLabel = coll.mLabel;
}

namespace
{
   constexpr float extraW = 10;
   constexpr float extraH = 58;
   constexpr float gridSquareDimension = 18;
   constexpr int maxGridSide = 20;
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
      SetSnapshot(mCurrentSnapshot, time);
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

void Snapshots::Resize(const float w, const float h)
{
   SetGridSize(MAX(w - extraW, 120), MAX(h - extraH, gridSquareDimension));
}

void Snapshots::SetGridSize(const float w, const float h)
{
   mGrid->SetDimensions(w, h);
   const int cols = MIN(w / gridSquareDimension, maxGridSide);
   const int rows = MIN(h / gridSquareDimension, maxGridSide);
   mGrid->SetGrid(cols, rows);
   const int oldSize = static_cast<int>(mSnapshotCollection.size());
   if (oldSize < static_cast<size_t>(cols) * rows)
   {
      mSnapshotCollection.resize(static_cast<size_t>(cols) * rows);
      for (int i = oldSize; i < static_cast<int>(mSnapshotCollection.size()); ++i)
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

   SetUpFromSaveData();
}

void Snapshots::SetUpFromSaveData()
{
   SetGridSize(mModuleSaveData.GetFloat("gridwidth"), mModuleSaveData.GetFloat("gridheight"));
   mAllowSetOnAudioThread = mModuleSaveData.GetBool("allow_set_on_audio_thread");
}

void Snapshots::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << static_cast<int>(mSnapshotCollection.size());
   for (auto& coll : mSnapshotCollection)
   {
      out << static_cast<int>(coll.mSnapshots.size());
      for (auto& snapshot : coll.mSnapshots)
      {
         out << snapshot.mControlPath;
         out << snapshot.mValue;
         out << snapshot.mHasLFO;
         snapshot.mLFOSettings.SaveState(out);
         out << snapshot.mGridCols;
         out << snapshot.mGridRows;
         assert(snapshot.mGridContents.size() == static_cast<size_t>(snapshot.mGridCols) * snapshot.mGridRows);
         for (float& mGridContent : snapshot.mGridContents)
            out << mGridContent;
         out << snapshot.mString;
      }
      out << coll.mLabel;
   }

   out << static_cast<int>(mSnapshotModules.size());
   for (const auto module : mSnapshotModules)
      out << module->Path();

   out << static_cast<int>(mSnapshotControls.size());
   for (const auto control : mSnapshotControls)
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
         snapshotData.mGridContents.resize(static_cast<size_t>(snapshotData.mGridCols) * snapshotData.mGridRows);
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
      auto module = TheSynth->FindModule(path);
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
      auto control = TheSynth->FindUIControl(path);
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
         if (intSliderRev >= 1)
         {
            int dummy;
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

Snapshots::Snapshot::Snapshot(IUIControl* control, const Snapshots* snapshots)
{
   auto context = IClickable::sPathSaveContext;
   IClickable::sPathSaveContext = snapshots->GetParent() ? snapshots->GetParent()->Path() + "~" : "";
   mControlPath = control->Path();
   IClickable::sPathSaveContext = context;

   mValue = control->GetValue();

   const auto slider = dynamic_cast<FloatSlider*>(control);
   if (slider)
   {
      const auto lfo = slider->GetLFO();
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

   const auto grid = dynamic_cast<UIGrid*>(control);
   if (grid)
   {
      mGridCols = grid->GetCols();
      mGridRows = grid->GetRows();
      mGridContents.resize(static_cast<size_t>(grid->GetCols()) * grid->GetRows());
      for (int col = 0; col < grid->GetCols(); ++col)
      {
         for (int row = 0; row < grid->GetRows(); ++row)
         {
            mGridContents[static_cast<size_t>(col) + static_cast<size_t>(row) * grid->GetCols()] = grid->GetVal(col, row);
         }
      }
   }

   const auto textEntry = dynamic_cast<TextEntry*>(control);
   if (textEntry && textEntry->GetTextEntryType() == kTextEntry_Text)
      mString = textEntry->GetText();
}
