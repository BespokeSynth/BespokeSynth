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
: mGrid(nullptr)
, mSaveButton(nullptr)
, mDrawSetPresetsCountdown(0)
, mBlending(false)
, mBlendTime(0)
, mBlendTimeSlider(nullptr)
, mCurrentPreset(-1)
, mCurrentPresetSelector(nullptr)
{
}

Presets::~Presets()
{
   TheTransport->RemoveAudioPoller(this);
}

void Presets::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(5,38,120,50,8,3, this);
   mSaveButton = new ClickButton(this,"save",50,3);
   mBlendTimeSlider = new FloatSlider(this,"blend ms",5,20,120,15,&mBlendTime,0,5000);
   mCurrentPresetSelector = new DropdownList(this,"preset",85,3,&mCurrentPreset);
   
   mSaveButton->SetShowing(false);

   for (int i=0; i<100; ++i)
      mCurrentPresetSelector->AddLabel(ofToString(i).c_str(), i);
   
   {
      mModuleCable = new PatchCableSource(this, kConnectionType_Special);
      ofColor color = IDrawableModule::GetColor(kModuleType_Other);
      color.a *= .3f;
      mModuleCable->SetColor(color);
      mModuleCable->SetManualPosition(15, 10);
      mModuleCable->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
      //mModuleCable->SetPatchCableDrawMode(kPatchCableDrawMode_HoverOnly);
      AddPatchCableSource(mModuleCable);
   }
   
   {
      mUIControlCable = new PatchCableSource(this, kConnectionType_UIControl);
      ofColor color = IDrawableModule::GetColor(kModuleType_Modulator);
      color.a *= .3f;
      mUIControlCable->SetColor(color);
      mUIControlCable->SetManualPosition(30, 10);
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
   mSaveButton->Draw();
   mBlendTimeSlider->Draw();
   mCurrentPresetSelector->Draw();
   
   int hover = mGrid->CurrentHover();
   if (hover != -1 && !mPresetCollection.empty())
   {
      assert(hover >= 0 && hover < mPresetCollection.size());
      DrawTextNormal(mPresetCollection[hover].mDescription,50,0);
   }

   ofVec2f pos = mGrid->GetCellPosition(hover % mGrid->GetCols(), hover / mGrid->GetCols()) + mGrid->GetPosition(true);
   float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
   float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();

   if (GetKeyModifiers() == kModifier_Shift)
   {
      ofPushStyle();
      ofSetColor(0, 0, 0);
      ofFill();
      ofRect(pos.x + xsize / 2 - 1, pos.y + 2, 2, ysize - 4, 0);
      ofRect(pos.x + 2, pos.y + ysize / 2 - 1, xsize - 4, 2, 0);
      ofPopStyle();
   }
}

void Presets::UpdateGridValues()
{
   mGrid->Clear();
   for (int i=0; i<mGrid->GetRows()*mGrid->GetCols();++i)
   {
      float val = 0;
      if (mPresetCollection[i].mPresets.empty() == false)
         val = .5f;
      if (i == mCurrentPreset)
         val = 1;
      mGrid->SetVal(i%mGrid->GetCols(), i/mGrid->GetCols(), val);
   }
}

void Presets::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (mGrid->TestClick(x, y, right, true))
   {
      float gridX,gridY;
      mGrid->GetPosition(gridX, gridY, true);
      GridCell cell = mGrid->GetGridCellAt(x-gridX,y-gridY);
      
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
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
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
   for (std::list<Preset>::const_iterator i=coll.mPresets.begin();
        i != coll.mPresets.end(); ++i)
   {
      auto context = IClickable::sPathLoadContext;
      IClickable::sPathLoadContext = GetParent() ? GetParent()->Path() + "~" : "";
      IUIControl* control = TheSynth->FindUIControl(i->mControlPath);
      IClickable::sPathLoadContext = context;

      if (control)
      {
         if (mBlendTime == 0 || i->mHasLFO)
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

   if (mPresetModules.size() < numModules || mPresetControls.size() < numControls)  //we removed something, clean up any presets that refer to it
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
   
   for (int i=0; i<mPresetControls.size(); ++i)
   {
      coll.mPresets.push_back(Preset(mPresetControls[i], this));
   }
   for (int i=0; i<mPresetModules.size(); ++i)
   {
      std::vector<IUIControl*> controls = mPresetModules[i]->GetUIControls();
      for (int j=0; j<controls.size(); ++j)
      {
         coll.mPresets.push_back(Preset(controls[j], this));
      }
   }
}

void Presets::Save()
{
   ofxJSONElement root;
   
   Json::Value& presets = root["presets"];
   for (unsigned int i=0; i<mPresetCollection.size(); ++i)
   {
      Json::Value& preset = presets[i];
      preset["description"] = mPresetCollection[i].mDescription;
      int j = 0;
      for (const auto& presetData : mPresetCollection[i].mPresets)
      {
         preset["controls"][j]["control"] = presetData.mControlPath;
         preset["controls"][j]["value"] = presetData.mValue;
         preset["controls"][j]["has_lfo"] = presetData.mHasLFO;
         if (presetData.mHasLFO)
         {
            preset["controls"][j]["lfo_interval"] = presetData.mLFOSettings.mInterval;
            preset["controls"][j]["lfo_osctype"] = presetData.mLFOSettings.mOscType;
            preset["controls"][j]["lfo_offset"] = presetData.mLFOSettings.mLFOOffset;
            preset["controls"][j]["lfo_bias"] = presetData.mLFOSettings.mBias;
            preset["controls"][j]["lfo_spread"] = presetData.mLFOSettings.mSpread;
            preset["controls"][j]["lfo_soften"] = presetData.mLFOSettings.mSoften;
            preset["controls"][j]["lfo_shuffle"] = presetData.mLFOSettings.mShuffle;
         }
         ++j;
      }
   }
   
   root.save(ofToDataPath(mModuleSaveData.GetString("presetsfile")), true);
}

namespace
{
   const float extraW = 10;
   const float extraH = 43;
   const int maxGridSide = 20;
}

void Presets::Load()
{
   mPresetCollection.clear();
   mPresetCollection.resize(maxGridSide * maxGridSide);
   
   std::string presetsFile = mModuleSaveData.GetString("presetsfile");
   if (!presetsFile.empty())
   {
      ofxJSONElement root;
      root.open(ofToDataPath(mModuleSaveData.GetString("presetsfile")));
      
      Json::Value& presets = root["presets"];
      for (int i=0; i<presets.size(); ++i)
      {
         try
         {
            Json::Value& preset = presets[i];
            mPresetCollection[i].mDescription = preset["description"].asString();
            mPresetCollection[i].mPresets.resize(preset["controls"].size());
            int j = 0;
            for (auto& presetData : mPresetCollection[i].mPresets)
            {
               presetData.mControlPath = preset["controls"][j]["control"].asString();
               presetData.mValue = preset["controls"][j]["value"].asDouble();
               presetData.mHasLFO = preset["controls"][j]["has_lfo"].asBool();
               if (presetData.mHasLFO)
               {
                  presetData.mLFOSettings.mInterval = (NoteInterval)preset["controls"][j]["lfo_interval"].asInt();
                  presetData.mLFOSettings.mOscType = (OscillatorType)preset["controls"][j]["lfo_osctype"].asInt();
                  presetData.mLFOSettings.mLFOOffset = preset["controls"][j]["lfo_offset"].asDouble();
                  presetData.mLFOSettings.mBias = preset["controls"][j]["lfo_bias"].asDouble();
                  presetData.mLFOSettings.mSpread = preset["controls"][j]["lfo_spread"].asDouble();
                  presetData.mLFOSettings.mSoften = preset["controls"][j]["lfo_soften"].asDouble();
                  presetData.mLFOSettings.mShuffle = preset["controls"][j]["lfo_shuffle"].asDouble();
               }
               ++j;
            }
         }
         catch (Json::LogicError& e)
         {
            TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
         }
      }
   }
   
   UpdateGridValues();
}

void Presets::ButtonClicked(ClickButton* button)
{
   if (button == mSaveButton)
      Save();
}

void Presets::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mCurrentPresetSelector)
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
   SetGridSize(w,h);
}

void Presets::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
   int cols = MIN(w / 15, maxGridSide);
   int rows = MIN(h / 15, maxGridSide);
   mGrid->SetGrid(cols, rows);
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
   mModuleSaveData.LoadString("presetsfile", moduleInfo);
   mModuleSaveData.LoadInt("defaultpreset", moduleInfo, -1, -1, 100, true);
   
   mModuleSaveData.LoadFloat("gridwidth", moduleInfo, 120, 120, 1000);
   mModuleSaveData.LoadFloat("gridheight", moduleInfo, 50, 15, 1000);
   
   SetUpFromSaveData();
}

void Presets::SetUpFromSaveData()
{
   Load();
   SetGridSize(mModuleSaveData.GetFloat("gridwidth"), mModuleSaveData.GetFloat("gridheight"));
}

namespace
{
   const int kSaveStateRev = 1;
}

void Presets::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
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

void Presets::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   int collSize;
   in >> collSize;
   mPresetCollection.resize(collSize);
   for (int i=0; i<collSize; ++i)
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
      }
      in >> mPresetCollection[i].mDescription;
   }
   
   UpdateGridValues();
   
   std::string path;
   int size;
   in >> size;
   for (int i=0; i<size; ++i)
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
   for (int i=0; i<size; ++i)
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

std::vector<IUIControl*> Presets::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mCurrentPresetSelector);
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
}

