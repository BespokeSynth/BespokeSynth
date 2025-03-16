
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

#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "FloatSliderLFOControl.h"
#include "Slider.h"
#include "Ramp.h"
#include "INoteReceiver.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "Push2Control.h"

class Snapshots : public IDrawableModule, public IButtonListener, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public INoteReceiver, public ITextEntryListener, public IPush2GridController
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
   bool IsResizable() const override { return mDisplayMode == DisplayMode::Grid; }
   void Resize(float w, float h) override;

   bool HasSnapshot(int index) const;
   int GetCurrentSnapshot() const { return mCurrentSnapshot; }
   bool IsTargetingModule(IDrawableModule* module) const;
   void AddSnapshotTarget(IDrawableModule* target);
   void SetSnapshot(int idx, double time);
   void StoreSnapshot(int idx, bool setAsCurrent);
   void DeleteSnapshot(int idx);

   void OnTransportAdvanced(float amount) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;
   int GetModuleSaveStateRev() const override { return 4; }
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;
   void UpdateOldControlName(std::string& oldName) override;

   static std::vector<IUIControl*> sSnapshotHighlightControls;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return true; }

private:
   void UpdateGridValues();
   void SetGridSize(float w, float h);
   bool IsConnectedToPath(std::string path) const;
   void RandomizeTargets();
   void RandomizeControl(IUIControl* control);
   void UpdateListGrid();
   void ResizeSnapshotCollection(int size);

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   enum class DisplayMode
   {
      Grid,
      List
   };

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

   struct SnapshotModuleData
   {
      SnapshotModuleData(IDrawableModule* module);
      std::string mModulePath;
      std::string mData;
   };

   struct SnapshotCollection
   {
      std::list<Snapshot> mSnapshots;
      std::list<SnapshotModuleData> mModuleData;
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
   TextEntry* mSnapshotLabelEntry{ nullptr };
   std::string mSnapshotLabel{};
   int mLoadRev{ -1 };
   bool mStoreMode{ false };
   Checkbox* mStoreCheckbox{ nullptr };
   bool mDeleteMode{ false };
   Checkbox* mDeleteCheckbox{ nullptr };
   bool mAutoStoreOnSwitch{ false };
   DisplayMode mDisplayMode{ DisplayMode::List };
   int mSnapshotRenameIndex{ -1 };
   float mOldWidth{ 0 };
   float mOldHeight{ 0 };
};
