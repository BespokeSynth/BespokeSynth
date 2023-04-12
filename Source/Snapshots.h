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
//  Snapshots.h
//  modularSynth
//
//  Created by Ryan Challinor on 7/29/13.
//
//

#pragma once

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
#include "DropdownList.h"
#include "TextEntry.h"

class Snapshots : public IDrawableModule, public IButtonListener, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public INoteReceiver, public ITextEntryListener
{
public:
   Snapshots();
   virtual ~Snapshots();
   static IDrawableModule* Create() { return new Snapshots(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

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

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;
   int GetModuleSaveStateRev() const override { return 3; }
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;
   void UpdateOldControlName(std::string& oldName) override;

   static std::vector<IUIControl*> sSnapshotHighlightControls;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return true; }

private:
   void SetSnapshot(int idx, double time);
   void Store(int idx);
   void Delete(int idx);
   void UpdateGridValues();
   void SetGridSize(float w, float h);
   bool IsConnectedToPath(std::string path) const;
   void RandomizeTargets();
   void RandomizeControl(IUIControl* control);

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   struct Snapshot
   {
      Snapshot() {}
      Snapshot(std::string path, float val)
      : mControlPath(path)
      , mValue(val)
      {}
      Snapshot(IUIControl* control, Snapshots* snapshots);
      bool operator==(const Snapshot& other) const
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

   struct SnapshotCollection
   {
      std::list<Snapshot> mSnapshots;
      std::string mLabel;
   };

   struct ControlRamp
   {
      IUIControl* mUIControl{ nullptr };
      Ramp mRamp;
   };

   UIGrid* mGrid{ nullptr };
   std::vector<SnapshotCollection> mSnapshotCollection;
   ClickButton* mRandomizeButton{ nullptr };
   ClickButton* mAddButton{ nullptr };
   int mDrawSetSnapshotCountdown{ 0 };
   std::vector<IDrawableModule*> mSnapshotModules{};
   std::vector<IUIControl*> mSnapshotControls{};
   bool mBlending{ false };
   float mBlendTime{ 0 };
   FloatSlider* mBlendTimeSlider{ nullptr };
   float mBlendProgress{ 0 };
   std::vector<ControlRamp> mBlendRamps;
   ofMutex mRampMutex;
   int mCurrentSnapshot{ 0 };
   DropdownList* mCurrentSnapshotSelector{ nullptr };
   PatchCableSource* mModuleCable{ nullptr };
   PatchCableSource* mUIControlCable{ nullptr };
   int mQueuedSnapshotIndex{ -1 };
   bool mAllowSetOnAudioThread{ false };
   bool mAutoStoreOnSwitch{ false };
   TextEntry* mSnapshotLabelEntry{ nullptr };
   std::string mSnapshotLabel;
   int mLoadRev{ -1 };
};
