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
//  Presets.h
//  modularSynth
//
//  Created by Ryan Challinor on 7/29/13.
//
//

#ifndef __modularSynth__Presets__
#define __modularSynth__Presets__

#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "FloatSliderLFOControl.h"
#include "Transport.h"
#include "Slider.h"
#include "Ramp.h"
#include "INoteReceiver.h"

class Presets : public IDrawableModule, public IButtonListener, public IAudioPoller, public IFloatSliderListener, public IIntSliderListener, public INoteReceiver
{
public:
   Presets();
   virtual ~Presets();
   static IDrawableModule* Create() { return new Presets(); }

   void CreateUIControls() override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   void OnTransportAdvanced(float amount) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;
   void UpdateOldControlName(std::string& oldName) override;

   static std::vector<IUIControl*> sPresetHighlightControls;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   void SetPreset(int idx);
   void Store(int idx);
   void UpdateGridValues();
   void SetGridSize(float w, float h);
   bool IsConnectedToPath(std::string path) const;
   void RandomizeTargets();
   void RandomizeControl(IUIControl* control);

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   struct Preset
   {
      Preset() {}
      Preset(std::string path, float val)
      : mControlPath(path)
      , mValue(val)
      {}
      Preset(IUIControl* control, Presets* presets);
      bool operator==(const Preset& other) const
      {
         return mControlPath == other.mControlPath &&
                mValue == other.mValue &&
                mHasLFO == other.mHasLFO;
      }
      std::string mControlPath;
      float mValue{ 0 };
      bool mHasLFO{ false };
      LFOSettings mLFOSettings;
      int mGridCols{ 0 };
      int mGridRows{ 0 };
      std::vector<float> mGridContents{};
      std::string mString;
   };

   struct PresetCollection
   {
      std::list<Preset> mPresets;
      std::string mDescription;
   };

   struct ControlRamp
   {
      IUIControl* mUIControl{ nullptr };
      Ramp mRamp;
   };

   UIGrid* mGrid{ nullptr };
   std::vector<PresetCollection> mPresetCollection;
   ClickButton* mRandomizeButton{ nullptr };
   ClickButton* mAddButton{ nullptr };
   int mDrawSetPresetsCountdown{ 0 };
   std::vector<IDrawableModule*> mPresetModules{};
   std::vector<IUIControl*> mPresetControls{};
   bool mBlending{ false };
   float mBlendTime{ 0 };
   FloatSlider* mBlendTimeSlider{ nullptr };
   float mBlendProgress{ 0 };
   std::vector<ControlRamp> mBlendRamps;
   ofMutex mRampMutex;
   int mCurrentPreset{ 0 };
   IntSlider* mCurrentPresetSlider{ nullptr };
   PatchCableSource* mModuleCable{ nullptr };
   PatchCableSource* mUIControlCable{ nullptr };
};


#endif /* defined(__modularSynth__Presets__) */
