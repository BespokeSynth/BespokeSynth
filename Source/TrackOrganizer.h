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

   int GetModuleIndex() const { return mModuleIndex; }
   void SetModuleIndex(int index) { mModuleIndex = index; }
   void AdjustModuleIndex(int amount);
   IDrawableModule* GetCurrentModule() const;
   Snapshots* GetSnapshots() const;
   IAbletonGridController* GetGridInterface() const;
   std::vector<IDrawableModule*> GetModuleList() const;
   IInputRecordable* GetRecorder() const;
   Amplifier* GetGain() const;
   int GetColorIndex() const { return mColorIndex; }
   std::string GetTrackName() const { return mTrackName; }

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
   void GetModuleDimensions(float& width, float& height) override;

   std::list<IDrawableModule*> GetAllTrackModules();
   bool ShouldShowCables() const;

   float mWidth{ 200 };
   float mHeight{ 68 };

   PatchCableSource* mSnapshotsCable{ nullptr };
   PatchCableSource* mGridInterfaceCable{ nullptr };
   std::array<PatchCableSource*, 5> mControlModuleCables{};
   PatchCableSource* mRecorderCable{ nullptr };
   PatchCableSource* mGainCable{ nullptr };
   PatchCableSource* mOtherTrackModulesCable{ nullptr };
   int mModuleIndex{ 0 };
   bool mSelectModulesOnMouseRelease{ false };
   bool mDrawTrackBounds{ true };

   TextEntry* mNameEntry{ nullptr };
   std::string mTrackName{ "track" };
   DropdownList* mColorSelector{ nullptr };
   int mColorIndex{ 0 };
   ClickButton* mSelectModulesButton{ nullptr };
};
