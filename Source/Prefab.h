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
/*
  ==============================================================================

    Prefab.h
    Created: 25 Sep 2016 10:14:15am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "ModuleContainer.h"

class PatchCableSource;
class SaveTarget;
class PrefabModulationPort;

enum class PrefabDisplayMode
{
   Edit,
   Presentation
};

class Prefab : public IDrawableModule, public IButtonListener, public IDropdownListener
{
public:
   Prefab();
   ~Prefab();
   static IDrawableModule* Create() { return new Prefab(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   std::string GetTitleLabel() const override;
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   ModuleContainer* GetContainer() override { return &mModuleContainer; }
   void Disband();
   void AddModule(IDrawableModule* module);
   void SavePrefab(const SaveTarget& target);

   void Poll() override;
   bool ShouldClipContents() override { return false; }

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   void LoadPrefab(std::string loadPath);

   static bool sLoadingPrefab;
   static bool sLastLoadWasPrefab;
   static IDrawableModule* sJustReleasedModule;

   bool IsEnabled() const override { return mEnabled; }

   PrefabDisplayMode GetDisplayMode() const { return mDisplayMode; }
   bool IsInterfaceModule(const IDrawableModule* module) const;
   void SetInterfaceModule(IDrawableModule* module, bool isInterface);
   //moves a module out of this prefab into the prefab's parent container. ports can't be removed, only deleted.
   bool CanRemoveModule(const IDrawableModule* module) const;
   void RemoveModule(IDrawableModule* module);
   //the prefab whose container directly holds this module, if any
   static Prefab* GetOwningPrefab(const IDrawableModule* module);
   //true if this module is inside a prefab that's currently hiding it for presentation mode
   static bool IsHiddenByPresentation(const IDrawableModule* module);
   static bool ShouldSuppressCableSource(const PatchCableSource* source);
   void OpenSaveDialog();

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;

   bool CanAddDropModules();
   bool IsAddableModule(IDrawableModule* module);
   bool IsMouseHovered();

   void UpdatePrefabName(std::string path);

   std::vector<IDrawableModule*> GetInterfaceModules() const;
   void SetPresentationApplied(bool apply);
   void UpdateHeaderControls();
   void LayOutPresentation(float topMargin);
   float GetWidestPortWidth(bool inlets) const;
   float GetPresentationLeftMargin() const;
   void SpawnPort(int index);
   void PruneInterfaceModules();
   void BridgePortConnections();
   bool BridgeModulationPort(PrefabModulationPort* port, const std::vector<IDrawableModule*>& allModules);
   ofxJSONElement WriteModulesWithEditPositions();
   void SavePresentationSettings(ofxJSONElement& info);
   void LoadPresentationSettings(const ofxJSONElement& info);
   void AddInterfaceModulesByName(const ofxJSONElement& names);

   ClickButton* mSaveButton{ nullptr };
   ClickButton* mLoadButton{ nullptr };
   ClickButton* mDisbandButton{ nullptr };
   ModuleContainer mModuleContainer;
   std::string mPrefabName{ "" };

   std::vector<IDrawableModule*> mInterfaceModules; //modules shown in presentation mode
   DropdownList* mDisplayModeDropdown{ nullptr };
   PrefabDisplayMode mDisplayMode{ PrefabDisplayMode::Edit };
   DropdownList* mAddPortDropdown{ nullptr };
   int mAddPortIndex{ -1 };

   bool mPresentationApplied{ false };
   //keyed by module rather than name, so renaming a module doesn't lose its positions. names are only used when saving.
   std::map<IDrawableModule*, ofVec2f> mEditPositions; //positions to restore when leaving presentation mode
   std::map<IDrawableModule*, ofVec2f> mPresentationPositions; //remembered interface module positions in presentation mode
   float mPresentationWidth{ 120 };
   float mPresentationHeight{ 40 };

   bool mLocked{ false };
};
