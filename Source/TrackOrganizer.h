/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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
//  TrackOrganizer.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/14/25.
//
//

#pragma once

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "TextEntry.h"
#include "DropdownList.h"
#include "ClickButton.h"

class Snapshots;
class IAbletonGridController;
class IInputRecordable;
class Amplifier;
class AudioSend;

class TrackOrganizer : public IDrawableModule, public ITextEntryListener, public IDropdownListener, public IButtonListener
{
public:
   TrackOrganizer();
   virtual ~TrackOrganizer();
   static IDrawableModule* Create() { return new TrackOrganizer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   void KeyPressed(int key, bool isRepeat) override;

   int GetModuleIndex() const { return mModuleIndex; }
   float GetModuleViewOffset() const { return mModuleViewOffset; }
   void ResetModuleIndex()
   {
      mModuleIndex = 0;
      mModuleViewOffset = 0.0f;
   }
   bool AdjustModuleIndex(int amount);
   void SetModuleViewOffset(float offset) { mModuleViewOffset = offset; }
   IDrawableModule* GetCurrentModule() const;
   IAbletonGridController* GetCurrentGridInterface() const;
   Snapshots* GetSnapshots() const;
   IUIControl* GetSoundSelector() const;
   int GetNumPages() const { return (int)mControlModuleCables.size(); }
   std::vector<IDrawableModule*> GetControlModules() const;
   std::vector<IAbletonGridController*> GetGridInterfaces() const;
   IInputRecordable* GetRecorder() const;
   Amplifier* GetGain() const;
   AudioSend* GetSend() const;
   int GetColorIndex() const { return mColorIndex; }
   std::string GetTrackName() const { return mTrackName; }
   ofRectangle GetBoundingRect();

   void TextEntryComplete(TextEntry* entry) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void PreDrawModuleUnclipped() override;
   void DrawModuleUnclipped() override;

   std::list<IDrawableModule*> GetAllTrackModules();
   bool ShouldShowCables() const;
   void GatherModules(const std::vector<IDrawableModule*>& modulesToAdd);

   PatchCableSource* mSnapshotsCable{ nullptr };
   PatchCableSource* mSoundSelectorCable{ nullptr };
   static constexpr int kNumPages{ 5 };
   std::array<PatchCableSource*, kNumPages> mControlModuleCables{};
   std::array<PatchCableSource*, kNumPages> mGridInterfaceCables{};
   PatchCableSource* mRecorderCable{ nullptr };
   PatchCableSource* mGainCable{ nullptr };
   PatchCableSource* mSendCable{ nullptr };
   PatchCableSource* mOtherTrackModulesCable{ nullptr };
   int mModuleIndex{ 0 };
   float mModuleViewOffset{ 0.0f };
   bool mSelectModulesOnMouseRelease{ false };
   bool mDrawTrackBounds{ true };
   bool mDrawTrackName{ true };

   TextEntry* mNameEntry{ nullptr };
   std::string mTrackName{ "track" };
   DropdownList* mColorSelector{ nullptr };
   int mColorIndex{ 0 };
   ClickButton* mSelectModulesButton{ nullptr };
};
