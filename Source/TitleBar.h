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
//  TitleBar.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/30/14.
//
//

#ifndef __Bespoke__TitleBar__
#define __Bespoke__TitleBar__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "WindowCloseListener.h"

class ModuleFactory;
class TitleBar;
class HelpDisplay;
struct SpawnListManager;
class PluginListWindow;

class SpawnList
{
public:
   SpawnList(IDropdownListener* owner, SpawnListManager* listManager, int x, int y, std::string label);
   void SetList(std::vector<std::string> spawnables, std::string overrideModuleType);
   void OnSelection(DropdownList* list);
   void SetPosition(int x, int y);
   void SetPositionRelativeTo(SpawnList* list);
   void Draw();
   DropdownList* GetList() { return mSpawnList; }
   IDrawableModule* Spawn();
   
private:
   std::string mLabel;
   std::vector<std::string> mSpawnables;
   int mSpawnIndex;
   DropdownList* mSpawnList;
   IDropdownListener* mOwner;
   SpawnListManager* mListManager;
   ofVec2f mPos;
   std::string mOverrideModuleType;
};

struct SpawnListManager
{
   SpawnListManager(IDropdownListener* owner);
   
   void SetModuleFactory(ModuleFactory* factory);
   void SetUpPrefabsDropdown();
   void SetUpVstDropdown();
   
   std::vector<SpawnList*> GetDropdowns() { return mDropdowns; }
   
   SpawnList mInstrumentModules;
   SpawnList mNoteModules;
   SpawnList mSynthModules;
   SpawnList mAudioModules;
   SpawnList mModulatorModules;
   SpawnList mPulseModules;
   SpawnList mOtherModules;
   SpawnList mVstPlugins;
   SpawnList mPrefabs;
   
private:
   std::vector<SpawnList*> mDropdowns;
};

class NewPatchConfirmPopup : public IDrawableModule, public IButtonListener
{
public:
   NewPatchConfirmPopup() {}
   void CreateUIControls() override;
   void DrawModule() override;
   bool HasTitleBar() const override { return false; }

   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }

   void ButtonClicked(ClickButton* button) override;
   
private:
   int mWidth;
   int mHeight;
   ClickButton* mConfirmButton;
   ClickButton* mCancelButton;
};

class TitleBar : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener, public WindowCloseListener
{
public:
   TitleBar();
   ~TitleBar();
   
   
   void CreateUIControls() override;
   bool HasTitleBar() const override { return false; }
   bool AlwaysOnTop() override { return true; }
   bool IsSingleton() const override { return true; }
   
   HelpDisplay* GetHelpDisplay() { return mHelpDisplay; }

   void SetModuleFactory(ModuleFactory* factory) { mSpawnLists.SetModuleFactory(factory); }
   void ListLayouts();
   void ManageVSTs();
   
   bool IsSaveable() override { return false; }

   void OnWindowClosed() override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

   static bool sShowInitialHelpOverlay;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   
   bool HiddenByZoom() const;
   float GetPixelWidth() const;
   
   ClickButton* mPlayPauseButton;
   ClickButton* mSaveLayoutButton;
   ClickButton* mResetLayoutButton;
   ClickButton* mSaveStateButton;
   ClickButton* mSaveStateAsButton;
   ClickButton* mLoadStateButton;
   ClickButton* mWriteAudioButton;
   DropdownList* mLoadLayoutDropdown;
   ClickButton* mDisplayHelpButton;
   ClickButton* mDisplayUserPrefsEditorButton;
   Checkbox* mEventLookaheadCheckbox;
   int mLoadLayoutIndex;
   Checkbox* mShouldAutosaveCheckbox;
   
   HelpDisplay* mHelpDisplay;
   
   SpawnListManager mSpawnLists;
   
   bool mLeftCornerHovered;

   std::unique_ptr<PluginListWindow> mPluginListWindow;

   NewPatchConfirmPopup mNewPatchConfirmPopup;
};

extern TitleBar* TheTitleBar;

#endif /* defined(__Bespoke__TitleBar__) */


