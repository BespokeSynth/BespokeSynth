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

#ifndef PREFAB_H_INCLUDED
#define PREFAB_H_INCLUDED

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "ModuleContainer.h"

class PatchCableSource;

class Prefab : public IDrawableModule, public IButtonListener
{
public:
   Prefab();
   ~Prefab();
   static IDrawableModule* Create() { return new Prefab(); }
   
   std::string GetTitleLabel() const override;
   void CreateUIControls() override;
   
   ModuleContainer* GetContainer() override { return &mModuleContainer; }
   
   void Poll() override;
   bool ShouldClipContents() override { return false; }
   
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void LoadPrefab(std::string loadPath);

   static bool sLoadingPrefab;
   static IDrawableModule* sJustReleasedModule;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   void MouseReleased() override;

   bool CanAddDropModules();
   bool IsMouseHovered();
   
   void SavePrefab(std::string savePath);
   void UpdatePrefabName(std::string path);
   
   PatchCableSource* mRemoveModuleCable;
   ClickButton* mSaveButton;
   ClickButton* mLoadButton;
   ClickButton* mDisbandButton;
   ModuleContainer mModuleContainer;
   std::string mPrefabName;
};



#endif  // PREFAB_H_INCLUDED
