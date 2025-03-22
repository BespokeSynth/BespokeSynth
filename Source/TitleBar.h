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

#pragma once

#include <memory>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "VSTPlugin.h"
#include "WindowCloseListener.h"
#include "ModuleFactory.h"

class ModuleFactory;
class TitleBar;
class HelpDisplay;
struct SpawnListManager;
class PluginListWindow;

class SpawnList
{
public:
   SpawnList(IDropdownListener* owner, int x, int y, std::string label, ModuleCategory moduleCategory, bool showDecorators);
   void SetList(std::vector<ModuleFactory::Spawnable> spawnables);
   void OnSelection(DropdownList* list);
   void SetPosition(int x, int y);
   void Draw();
   DropdownList* GetList() { return mSpawnList; }
   IDrawableModule* Spawn(int index);
   std::string GetLabel() const { return mLabel; }
   ModuleCategory GetCategory() const { return mModuleCategory; }
   const std::vector<ModuleFactory::Spawnable>& GetElements() const { return mSpawnables; }
   bool ShouldShowDecorators() const { return mShowDecorators; }

private:
   std::string mLabel;
   std::vector<ModuleFactory::Spawnable> mSpawnables;
   int mSpawnIndex{ -1 };
   DropdownList* mSpawnList{ nullptr };
   IDropdownListener* mOwner{ nullptr };
   ofVec2f mPos;
   ModuleCategory mModuleCategory;
   bool mShowDecorators{ false };
};

struct SpawnListManager
{
   SpawnListManager(IDropdownListener* owner);

   void SetModuleFactory(ModuleFactory* factory);
   void SetUpPrefabsDropdown();
   void SetUpPluginsDropdown();

   const std::vector<SpawnList*>& GetDropdowns() const { return mDropdowns; }

   SpawnList mInstrumentModules;
   SpawnList mNoteModules;
   SpawnList mSynthModules;
   SpawnList mAudioModules;
   SpawnList mModulatorModules;
   SpawnList mPulseModules;
   SpawnList mOtherModules;
   SpawnList mPlugins;
   SpawnList mPrefabs;

private:
   std::vector<SpawnList*> mDropdowns;
   juce::PluginDescription stump{};
};

class NewPatchConfirmPopup : public IDrawableModule, public IButtonListener
{
public:
   NewPatchConfirmPopup() {}
   void CreateUIControls() override;
   void DrawModule() override;
   bool HasTitleBar() const override { return false; }

   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void ButtonClicked(ClickButton* button, double time) override;

private:
   int mWidth{ 200 };
   int mHeight{ 20 };
   ClickButton* mConfirmButton{ nullptr };
   ClickButton* mCancelButton{ nullptr };
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
   void Poll() override;

   HelpDisplay* GetHelpDisplay() { return mHelpDisplay; }

   void SetModuleFactory(ModuleFactory* factory) { mSpawnLists.SetModuleFactory(factory); }
   void ListLayouts();
   void ManagePlugins();
   const std::vector<SpawnList*>& GetSpawnLists() const { return mSpawnLists.GetDropdowns(); }
   void DisplayTemporaryMessage(std::string message);

   bool IsSaveable() override { return false; }

   void OnWindowClosed() override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   static bool sShowInitialHelpOverlay;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   bool HiddenByZoom() const;
   float GetPixelWidth() const;

   ClickButton* mPlayPauseButton{ nullptr };
   ClickButton* mSaveLayoutButton{ nullptr };
   ClickButton* mResetLayoutButton{ nullptr };
   ClickButton* mSaveStateButton{ nullptr };
   ClickButton* mSaveStateAsButton{ nullptr };
   ClickButton* mLoadStateButton{ nullptr };
   ClickButton* mWriteAudioButton{ nullptr };
   DropdownList* mLoadLayoutDropdown{ nullptr };
   ClickButton* mDisplayHelpButton{ nullptr };
   ClickButton* mDisplayUserPrefsEditorButton{ nullptr };
   ClickButton* mHomeButton{ nullptr };
   Checkbox* mEventLookaheadCheckbox{ nullptr };
   int mLoadLayoutIndex{ -1 };
   Checkbox* mShouldAutosaveCheckbox{ nullptr };

   HelpDisplay* mHelpDisplay{ nullptr };

   SpawnListManager mSpawnLists;

   bool mLeftCornerHovered{ false };

   std::unique_ptr<PluginListWindow> mPluginListWindow;

   NewPatchConfirmPopup mNewPatchConfirmPopup;

   std::string mDisplayMessage;
   double mDisplayMessageTime;
};

extern TitleBar* TheTitleBar;
