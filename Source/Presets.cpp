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
//  Presets.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 7/29/13.
//
//

#include "Presets.h"
#include "ModularSynth.h"
#include "Slider.h"
#include "ofxJSONElement.h"
#include "PatchCableSource.h"

std::vector<IUIControl*> Presets::sPresetHighlightControls;

Presets::Presets()
{
}

Presets::~Presets()
{
   TheTransport->RemoveAudioPoller(this);
}

void Presets::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid("uigrid", 5, 38, 120, 50, 8, 3, this);
   mBlendTimeSlider = new FloatSlider(this, "blend", 5, 20, 70, 15, &mBlendTime, 0, 5000);
   mCurrentPresetSlider = new IntSlider(this, "preset", 35, 3, 64, 15, &mCurrentPreset, 0, mGrid->GetCols() * mGrid->GetRows() - 1);
   mRandomizeButton = new ClickButton(this, "random", 78, 20);
   mAddButton = new ClickButton(this, "add", 101, 3);

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
}

void Presets::Init()
{
   IDrawableModule::Init();

   int defaultPreset = mModuleSaveData.GetInt("defaultpreset");
   if (defaultPreset != -1)
      SetPreset(defaultPreset);

   TheTransport->AddAudioPoller(this);
}

void Presets::Poll()
{
   if (mDrawSetPresetsCountdown > 0)
   {
      --mDrawSetPresetsCountdown;
      if (mDrawSetPresetsCountdown == 0)
         sPresetHighlightControls.clear();
   }

   if (!mBlending && !mBlendRamps.empty())
   {
      mBlendRamps.clear();
   }
}

void Presets::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGrid->Draw();
   mBlendTimeSlider->Draw();
   mCurrentPresetSlider->Draw();
   mRandomizeButton->Draw();
   mAddButton->Draw();

   int hover = mGrid->CurrentHover();
   if (hover != -1 && !mPresetCollection.empty())
   {
      assert(hover >= 0 && hover < mPresetCollection.size());
      DrawTextNormal(mPresetCollection[hover].mDescription, 50, 0);
   }

   bool shiftHeld = GetKeyModifiers() == kModifier_Shift;
   if (shiftHeld)
   {
      ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
      float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
      float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();

      ofPushStyle();
      ofSetColor(0, 0, 0);
      ofFill();
      ofRect(pos.x + xsize / 2 - 1, pos.y + 3, 2, ysize - 6, 0);
      ofRect(pos.x + 3, pos.y + ysize / 2 - 1, xsize - 6, 2, 0);
      ofPopStyle();
   }

   if (!shiftHeld)
   {
      ofVec2f pos = mGrid->GetCellPosition(mCurrentPreset % mGrid->GetCols(), mCurrentPreset / mGrid->GetCols()) + mGrid->GetPosition(true);
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

void Presets::UpdateGridValues()
{
   mGrid->Clear();
   assert(mPresetCollection.size() >= size_t(mGrid->GetRows()) * mGrid->GetCols());
   for (int i = 0; i < mGrid->GetRows() * mGrid->GetCols(); ++i)
   {
      float val = 0;
      if (mPresetCollection[i].mPresets.empty() == false)
         val = .5f;
      mGrid->SetVal(i % mGrid->GetCols(), i / mGrid->GetCols(), val);
   }
}

void Presets::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (mGrid->TestClick(x, y, right, true))
   {
      float gridX, gridY;
      mGrid->GetPosition(gridX, gridY, true);
      GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);

      mCurrentPreset = cell.mCol + cell.mRow * mGrid->GetCols();

      if (GetKeyModifiers() == kModifier_Shift)
         Store(mCurrentPreset);
      else
         SetPreset(mCurrentPreset);

      UpdateGridValues();
   }
}

bool Presets::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   return false;
}

void Presets::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (pitch < (int)mPresetCollection.size())
   {
      mCurrentPreset = pitch;
      SetPreset(pitch);
      UpdateGridValues();
   }
}

void Presets::SetPreset(int idx)
{
   assert(idx >= 0 && idx < mPresetCollection.size());

   if (mBlendTime > 0)
   {
      mRampMutex.lock();
      mBlending = true;
      mBlendProgress = 0;
      mBlendRamps.clear();
   }

   const PresetCollection& coll = mPresetCollection[idx];
   for (std::list<Preset>::const_iterator i = coll.mPresets.begin();
        i != coll.mPresets.end(); ++i)
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
            control->SetValueDirect(i->mValue);

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

         sPresetHighlightControls.push_back(control);
      }
   }

   if (mBlendTime > 0)
   {
      mRampMutex.unlock();
   }

   mDrawSetPresetsCountdown = 30;
}

void Presets::RandomizeTargets()
{
   for (int i = 0; i < mPresetControls.size(); ++i)
      RandomizeControl(mPresetControls[i]);
   for (int i = 0; i < mPresetModules.size(); ++i)
   {
      for (auto* control : mPresetModules[i]->GetUIControls())
         RandomizeControl(control);
   }
}

void Presets::RandomizeControl(IUIControl* control)
{
   if (strcmp(control->Name(), "enabled") == 0) //don't randomize enabled/disable checkbox, too annoying
      return;
   if (dynamic_cast<ClickButton*>(control) != nullptr)
      return;
   control->SetFromMidiCC(ofRandom(1), true);
}

void Presets::OnTransportAdvanced(float amount)
{
   if (mBlending)
   {
      mRampMutex.lock();

      mBlendProgress += amount * TheTransport->MsPerBar();

      for (auto& ramp : mBlendRamps)
      {
         ramp.mUIControl->SetValueDirect(ramp.mRamp.Value(mBlendProgress));
      }

      if (mBlendProgress >= mBlendTime)
         mBlending = false;

      mRampMutex.unlock();
   }
}

void Presets::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (TheSynth->IsLoadingState())
      return;

   auto numModules = mPresetModules.size();
   auto numControls = mPresetControls.size();

   mPresetModules.clear();
   for (auto cable : mModuleCable->GetPatchCables())
      mPresetModules.push_back(static_cast<IDrawableModule*>(cable->GetTarget()));
   mPresetControls.clear();
   for (auto cable : mUIControlCable->GetPatchCables())
      mPresetControls.push_back(static_cast<IUIControl*>(cable->GetTarget()));

   if (mPresetModules.size() < numModules || mPresetControls.size() < numControls) //we removed something, clean up any presets that refer to it
   {
      for (auto& square : mPresetCollection)
      {
         std::vector<Preset> toRemove;
         for (const auto& preset : square.mPresets)
         {
            if (!IsConnectedToPath(preset.mControlPath))
               toRemove.push_back(preset);
         }

         for (auto remove : toRemove)
            square.mPresets.remove(remove);
      }
   }
}

bool Presets::IsConnectedToPath(std::string path) const
{
   auto context = IClickable::sPathLoadContext;
   IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
   IUIControl* control = TheSynth->FindUIControl(path);
   IClickable::sPathLoadContext = context;

   if (control == nullptr)
      return false;

   if (VectorContains(control, mPresetControls))
      return true;

   if (VectorContains(control->GetModuleParent(), mPresetModules))
      return true;

   return false;
}

void Presets::Store(int idx)
{
   assert(idx >= 0 && idx < mPresetCollection.size());

   PresetCollection& coll = mPresetCollection[idx];
   coll.mPresets.clear();

   for (int i = 0; i < mPresetControls.size(); ++i)
   {
      coll.mPresets.push_back(Preset(mPresetControls[i], this));
   }
   for (int i = 0; i < mPresetModules.size(); ++i)
   {
      for (auto* control : mPresetModules[i]->GetUIControls())
      {
         if (dynamic_cast<ClickButton*>(control) == nullptr)
            coll.mPresets.push_back(Preset(control, this));
      }
      for (auto* grid : mPresetModules[i]->GetUIGrids())
         coll.mPresets.push_back(Preset(grid, this));
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 43;
   const int maxGridSide = 20;
}

void Presets::ButtonClicked(ClickButton* button)
{
   if (button == mRandomizeButton)
      RandomizeTargets();

   if (button == mAddButton)
   {
      for (size_t i = 0; i < mPresetCollection.size(); ++i)
      {
         if (mPresetCollection[i].mPresets.empty())
         {
            Store(i);
            mCurrentPreset = i;
            UpdateGridValues();
            break;
         }
      }
   }
}

void Presets::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCurrentPresetSlider)
   {
      SetPreset(mCurrentPreset);
      UpdateGridValues();
   }
}

void Presets::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void Presets::Resize(float w, float h)
{
   w = MAX(w - extraW, 120);
   h = MAX(h - extraH, 15);
   SetGridSize(w, h);
}

void Presets::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
   int cols = MIN(w / 15, maxGridSide);
   int rows = MIN(h / 15, maxGridSide);
   mGrid->SetGrid(cols, rows);
   if (mPresetCollection.size() < size_t(cols) * rows)
      mPresetCollection.resize(size_t(cols) * rows);
   mCurrentPresetSlider->SetExtents(0, mGrid->GetCols() * mGrid->GetRows() - 1);
   UpdateGridValues();
}

void Presets::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);

   moduleInfo["gridwidth"] = mGrid->GetWidth();
   moduleInfo["gridheight"] = mGrid->GetHeight();
}

void Presets::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("defaultpreset", moduleInfo, -1, -1, 100, true);

   mModuleSaveData.LoadFloat("gridwidth", moduleInfo, 120, 120, 1000);
   mModuleSaveData.LoadFloat("gridheight", moduleInfo, 50, 15, 1000);

   SetUpFromSaveData();
}

void Presets::SetUpFromSaveData()
{
   SetGridSize(mModuleSaveData.GetFloat("gridwidth"), mModuleSaveData.GetFloat("gridheight"));
}

void Presets::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mPresetCollection.size();
   for (auto& coll : mPresetCollection)
   {
      out << (int)coll.mPresets.size();
      for (auto& preset : coll.mPresets)
      {
         out << preset.mControlPath;
         out << preset.mValue;
         out << preset.mHasLFO;
         preset.mLFOSettings.SaveState(out);
         out << preset.mGridCols;
         out << preset.mGridRows;
         assert(preset.mGridContents.size() == size_t(preset.mGridCols) * preset.mGridRows);
         for (size_t i = 0; i < preset.mGridContents.size(); ++i)
            out << preset.mGridContents[i];
         out << preset.mString;
      }
      out << coll.mDescription;
   }

   out << (int)mPresetModules.size();
   for (auto module : mPresetModules)
      out << module->Path();

   out << (int)mPresetControls.size();
   for (auto control : mPresetControls)
      out << control->Path();
}

void Presets::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int collSize;
   in >> collSize;
   mPresetCollection.resize(collSize);
   for (int i = 0; i < collSize; ++i)
   {
      int presetSize;
      in >> presetSize;
      mPresetCollection[i].mPresets.resize(presetSize);
      int j = 0;
      for (auto& presetData : mPresetCollection[i].mPresets)
      {
         in >> presetData.mControlPath;
         in >> presetData.mValue;
         in >> presetData.mHasLFO;
         presetData.mLFOSettings.LoadState(in);
         in >> presetData.mGridCols;
         in >> presetData.mGridRows;
         // Check if the loaded values are within an accaptable range.
         // This is done because mGridCols and mGridRows could previously be saved with random values since they were not properly initialized.
         if (presetData.mGridCols < 0 || presetData.mGridCols > maxGridSide)
            presetData.mGridCols = 0;
         if (presetData.mGridRows < 0 || presetData.mGridRows > maxGridSide)
            presetData.mGridRows = 0;
         presetData.mGridContents.resize(size_t(presetData.mGridCols) * presetData.mGridRows);
         for (int k = 0; k < presetData.mGridCols * presetData.mGridRows; ++k)
            in >> presetData.mGridContents[k];
         in >> presetData.mString;
      }
      in >> mPresetCollection[i].mDescription;
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
         mPresetModules.push_back(module);
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
         mPresetControls.push_back(control);
         mUIControlCable->AddPatchCable(control);
      }
   }
}

void Presets::UpdateOldControlName(std::string& oldName)
{
   IDrawableModule::UpdateOldControlName(oldName);

   if (oldName == "blend ms")
      oldName = "blend";
}

std::vector<IUIControl*> Presets::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mCurrentPresetSlider);
   return ignore;
}

Presets::Preset::Preset(IUIControl* control, Presets* presets)
{
   auto context = IClickable::sPathSaveContext;
   IClickable::sPathSaveContext = presets->GetParent() ? presets->GetParent()->Path() + "~" : "";
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
