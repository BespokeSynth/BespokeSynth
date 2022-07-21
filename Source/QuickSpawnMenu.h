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

    QuickSpawnMenu.h
    Created: 22 Oct 2017 7:49:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "ModuleFactory.h"

class QuickSpawnMenu : public IDrawableModule
{
public:
   QuickSpawnMenu();
   virtual ~QuickSpawnMenu();

   void Init() override;
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void SetDimensions(int w, int h)
   {
      mWidth = w;
      mHeight = h;
   }
   bool HasTitleBar() const override { return false; }
   bool IsSaveable() override { return false; }
   std::string GetHoveredModuleTypeName();

   void ShowSpawnCategoriesPopup();

   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   void MouseReleased() override;

   bool IsSingleton() const override { return true; }

private:
   const ModuleFactory::Spawnable* GetElementAt(int x, int y) const;
   int GetIndexAt(int x, int y) const;
   void UpdateDisplay();
   void OnSelectItem(int index);
   void MoveMouseToIndex(int index);
   void ResetAppearPos();
   void UpdatePosition();

   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   enum class MenuMode
   {
      SingleLetter,
      ModuleCategories,
      SingleCategory,
      Search
   };

   float mWidth{ 200 };
   float mHeight{ 20 };
   int mLastHoverX{ 0 };
   int mLastHoverY{ 0 };
   juce::String mHeldKeys;
   ofVec2f mAppearAtMousePos;
   std::vector<ModuleFactory::Spawnable> mElements;
   int mHighlightIndex{ -1 };
   MenuMode mMenuMode{ MenuMode::SingleLetter };
   int mSelectedCategoryIndex{ -1 };
   juce::String mSearchString;
   float mScrollOffset{ 0 };
};

extern QuickSpawnMenu* TheQuickSpawnMenu;
