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

#include <iostream>
#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "TextEntry.h"

class SongBuilder : public IDrawableModule, public IButtonListener, public IDropdownListener, public IIntSliderListener, public ITimeListener, public IPulseReceiver, public ITextEntryListener, public INoteReceiver
{
public:
   SongBuilder();
   virtual ~SongBuilder();
   static IDrawableModule* Create() { return new SongBuilder(); }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendPressure(int pitch, int pressure) override {}
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   bool IsResizable() const override { return false; }
   void PostRepatch(PatchCableSource* cable, bool fromUserClick) override;

   void OnStep(double time, float velocity, int flags);
   void SetActiveSection(double time, int newSection);
   void DuplicateSection(int sectionIndex);
   void AddTarget();

   struct ControlTarget
   {
      void CreateUIControls(SongBuilder* owner);
      void Draw(float x, float y, int numRows);
      void TargetControlUpdated();
      IUIControl* GetTarget() const;
      void CleanUp();

      PatchCableSource* mCable{ nullptr };
      bool mIsCheckbox{ false };
      ClickButton* mMoveLeftButton{ nullptr };
      ClickButton* mMoveRightButton{ nullptr };
   };

   struct ControlValue
   {
      void CreateUIControls(SongBuilder* owner);
      void Draw(float x, float y, int sectionIndex, int targetIndex);
      void CleanUp();

      float mValue{ 0 };
      TextEntry* mValueEntry{ nullptr };
      bool mBoolValue{ false };
      Checkbox* mCheckbox{ nullptr };
      int mId{ -1 };
   };

   struct SongSection
   {
      explicit SongSection(std::string name)
      : mName(name)
      {}
      void CreateUIControls(SongBuilder* owner);
      void Draw(float x, float y, int sectionIndex, bool isCurrentSection);
      void TargetControlUpdated(SongBuilder::ControlTarget* target, int targetIndex, bool wasManuallyPatched);
      void AddValue(SongBuilder* owner);
      void MoveValue(int index, int amount);
      float GetWidth() const;
      void CleanUp();

      enum class ContextMenuItems
      {
         kNone,
         kDuplicate,
         kDelete,
         kMoveUp,
         kMoveDown
      };

      std::string mName{};
      TextEntry* mNameEntry{ nullptr };
      ClickButton* mActivateButton{ nullptr };
      DropdownList* mContextMenu{ nullptr };
      ContextMenuItems mContextMenuSelection{ ContextMenuItems::kNone };
      std::vector<ControlValue*> mValues{};
      int mId{ -1 };
   };

   int mCurrentSection{ -1 };

   NoteInterval mInterval{ NoteInterval::kInterval_1n };
   DropdownList* mIntervalSelector{ nullptr };

   std::vector<SongSection*> mSections{};
   std::vector<ControlTarget*> mTargets{};
   ClickButton* mAddTargetButton{ nullptr };
};
