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

    ModuleContainer.h
    Created: 16 Oct 2016 3:47:41pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "OpenFrameworksPort.h"
#include "IDrawableModule.h"
#include "ofxJSONElement.h"

class ModuleContainer
{
public:
   ModuleContainer();

   const std::vector<IDrawableModule*>& GetModules() const { return mModules; }

   void SetOwner(IDrawableModule* owner) { mOwner = owner; }
   IDrawableModule* GetOwner() const { return mOwner; }
   void DrawContents();
   void DrawModules();
   void DrawPatchCables(bool parentMinimized, bool inFront);
   void DrawUnclipped();
   void PostRender();
   void Poll();
   void Clear();
   void Exit();
   ofVec2f GetOwnerPosition() const;
   ofVec2f GetDrawOffset();
   ofVec2f& GetDrawOffsetRef() { return mDrawOffset; }
   void SetDrawOffset(ofVec2f offset) { mDrawOffset = offset; }
   float GetDrawScale() const;
   void SetDrawScale(float scale) { mDrawScale = scale; }

   void KeyPressed(int key, bool isRepeat);
   void KeyReleased(int key);
   void MouseMoved(float x, float y);
   void MouseReleased();
   IDrawableModule* GetModuleAt(float x, float y);
   void GetModulesWithinRect(ofRectangle rect, std::vector<IDrawableModule*>& output, bool ignorePinned = false);
   void MoveToFront(IDrawableModule* module);
   void AddModule(IDrawableModule* module);
   void TakeModule(IDrawableModule* module);
   void DeleteModule(IDrawableModule* module, bool fail = true);
   static void DeleteCablesForControl(const IUIControl* control);
   IDrawableModule* FindModule(std::string name, bool fail = true);
   IUIControl* FindUIControl(std::string path);
   bool IsHigherThan(IDrawableModule* checkFor, IDrawableModule* checkAgainst) const;
   void GetAllModules(std::vector<IDrawableModule*>& out);

   template <class T>
   std::vector<std::string> GetModuleNames()
   {
      std::vector<std::string> ret;
      for (int i = 0; i < mModules.size(); ++i)
      {
         if (dynamic_cast<T>(mModules[i]))
            ret.push_back(mModules[i]->Name());
      }
      return ret;
   }

   void LoadModules(const ofxJSONElement& modules);
   ofxJSONElement WriteModules();
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

   static constexpr int GetModuleSeparatorLength() { return 13; }
   static const char* GetModuleSeparator() { return "ryanchallinor"; }
   static bool DoesModuleHaveMoreSaveData(FileStreamIn& in);

private:
   std::vector<IDrawableModule*> mModules;
   IDrawableModule* mOwner{ nullptr };

   ofVec2f mDrawOffset;
   float mDrawScale{ 1 };
};
