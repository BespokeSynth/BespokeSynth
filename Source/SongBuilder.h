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
//  SongBuilder.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/05/22.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "TextEntry.h"
#include "Push2Control.h"

class SongBuilder : public IDrawableModule, public IButtonListener, public IDropdownListener, public IIntSliderListener, public ITimeListener, public IPulseReceiver, public ITextEntryListener, public INoteReceiver, public IPush2GridController
{
public:
   SongBuilder();
   virtual ~SongBuilder();
   static IDrawableModule* Create() { return new SongBuilder(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void Poll() override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendPressure(int pitch, int pressure) override {}
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;
   bool DrawToPush2Screen() override;

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool IsResizable() const override { return false; }
   void PostRepatch(PatchCableSource* cable, bool fromUserClick) override;
   bool ShouldSavePatchCableSources() const override { return false; }

   void SetActiveScene(double time, int newScene);
   void SetActiveSceneById(double time, int newSceneId);
   void DuplicateScene(int sceneIndex);
   void AddTarget();
   bool ShowSongSequencer() const { return mUseSequencer; }
   void RefreshSequencerDropdowns();
   void PlaySequence(double time, int startIndex);

   enum class ContextMenuItems
   {
      kNone,
      kDuplicate,
      kDelete,
      kMoveUp,
      kMoveDown
   };

   struct ControlTarget
   {
      void CreateUIControls(SongBuilder* owner);
      void Draw(float x, float y, int numRows);
      void TargetControlUpdated();
      IUIControl* GetTarget() const;
      void CleanUp();
      ofColor GetColor() { return mOwner->mColors[mColorIndex].color; }

      enum class DisplayType
      {
         TextEntry,
         Checkbox,
         Dropdown,
         NumDisplayTypes
      };

      SongBuilder* mOwner;
      PatchCableSource* mCable{ nullptr };
      DisplayType mDisplayType{ DisplayType::TextEntry };
      ClickButton* mMoveLeftButton{ nullptr };
      ClickButton* mMoveRightButton{ nullptr };
      ClickButton* mCycleDisplayTypeButton{ nullptr };
      DropdownList* mColorSelector{ nullptr };
      bool mHadTarget{ false };
      int mColorIndex{ 0 };
      int mId{ -1 };
   };

   struct ControlValue
   {
      void CreateUIControls(SongBuilder* owner);
      void Draw(float x, float y, int sceneIndex, ControlTarget* target);
      void CleanUp();
      void UpdateDropdownContents(ControlTarget* target);

      float mFloatValue{ 0 };
      TextEntry* mValueEntry{ nullptr };
      bool mBoolValue{ false };
      Checkbox* mCheckbox{ nullptr };
      int mIntValue{ 0 };
      DropdownList* mValueSelector{ nullptr };
      int mId{ -1 };
   };

   struct SongScene
   {
      explicit SongScene(std::string name)
      : mName(name)
      {}
      void CreateUIControls(SongBuilder* owner);
      void Draw(SongBuilder* owner, float x, float y, int sceneIndex);
      void TargetControlUpdated(SongBuilder::ControlTarget* target, int targetIndex, bool wasManuallyPatched);
      void AddValue(SongBuilder* owner);
      void MoveValue(int index, int amount);
      float GetWidth() const;
      void CleanUp();

      std::string mName{};
      TextEntry* mNameEntry{ nullptr };
      ClickButton* mActivateButton{ nullptr };
      DropdownList* mContextMenu{ nullptr };
      ContextMenuItems mContextMenuSelection{ ContextMenuItems::kNone };
      std::vector<ControlValue*> mValues{};
      int mId{ -1 };
   };

   struct TargetColor
   {
      TargetColor(std::string _name, ofColor _color)
      {
         name = _name;
         color = _color;
      }
      std::string name;
      ofColor color;
   };

   int mCurrentScene{ -1 };
   int mQueuedScene{ -1 };
   int mSequenceStepIndex{ -1 };
   int mSequenceStartStepIndex{ 0 };
   bool mSequenceStartQueued{ false };
   bool mSequencePaused{ false };
   bool mWantResetClock{ false };
   bool mJustResetClock{ false };
   bool mWantRefreshValueDropdowns{ false };
   std::vector<TargetColor> mColors{};

   static const int kMaxSequencerScenes = 128;
   static const int kSequenceEndId = -1;

   bool mUseSequencer{ false };
   Checkbox* mUseSequencerCheckbox{ nullptr };
   bool mResetOnSceneChange{ true };
   bool mActivateFirstSceneOnStop{ true };
   Checkbox* mActivateFirstSceneOnStopCheckbox{ nullptr };
   NoteInterval mChangeQuantizeInterval{ NoteInterval::kInterval_1n };
   DropdownList* mChangeQuantizeSelector{ nullptr };
   ClickButton* mAddTargetButton{ nullptr };
   ClickButton* mPlaySequenceButton{ nullptr };
   ClickButton* mStopSequenceButton{ nullptr };
   ClickButton* mPauseSequenceButton{ nullptr };
   bool mLoopSequence{ false };
   Checkbox* mLoopCheckbox{ nullptr };
   int mSequenceLoopStartIndex{ 0 };
   TextEntry* mSequenceLoopStartEntry{ nullptr };
   int mSequenceLoopEndIndex{ 0 };
   TextEntry* mSequenceLoopEndEntry{ nullptr };
   std::array<int, kMaxSequencerScenes> mSequencerSceneId{};
   std::array<DropdownList*, kMaxSequencerScenes> mSequencerSceneSelector{};
   std::array<int, kMaxSequencerScenes> mSequencerStepLength{};
   std::array<TextEntry*, kMaxSequencerScenes> mSequencerStepLengthEntry{};
   std::array<DropdownList*, kMaxSequencerScenes> mSequencerContextMenu{};
   std::array<ContextMenuItems, kMaxSequencerScenes> mSequencerContextMenuSelection{};
   std::array<ClickButton*, kMaxSequencerScenes> mSequencerPlayFromButton{};

   std::vector<SongScene*> mScenes{};
   std::vector<ControlTarget*> mTargets{};
};
