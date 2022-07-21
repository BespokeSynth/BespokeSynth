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

    QuickSpawnMenu.cpp
    Created: 22 Oct 2017 7:49:17pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "QuickSpawnMenu.h"
#include "ModularSynth.h"
#include "ModuleFactory.h"
#include "TitleBar.h"

#include "juce_gui_basics/juce_gui_basics.h"

QuickSpawnMenu* TheQuickSpawnMenu = nullptr;

namespace
{
   const int kItemSpacing = 15;
   const int kRightClickShiftX = 11;
   ofVec2f kModuleGrabOffset(-40, 10);
}

QuickSpawnMenu::QuickSpawnMenu()
{
   assert(TheQuickSpawnMenu == nullptr);
   TheQuickSpawnMenu = this;
}

QuickSpawnMenu::~QuickSpawnMenu()
{
   assert(TheQuickSpawnMenu == this);
   TheQuickSpawnMenu = nullptr;
}

void QuickSpawnMenu::Init()
{
   IDrawableModule::Init();
   SetShouldDrawOutline(false);
   SetShowing(false);
}

void QuickSpawnMenu::ShowSpawnCategoriesPopup()
{
   ResetAppearPos();
   mMenuMode = MenuMode::ModuleCategories;
   mSearchString = "";
   UpdateDisplay();
}

void QuickSpawnMenu::ResetAppearPos()
{
   mAppearAtMousePos.set(TheSynth->GetMouseX(GetOwningContainer()), TheSynth->GetMouseY(GetOwningContainer()));
   mScrollOffset = 0;
}

void QuickSpawnMenu::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);

   if (isRepeat)
      return;

   if (!IsShowing())
      ResetAppearPos();

   if ((!IsShowing() || mMenuMode == MenuMode::SingleLetter) && key >= 0 && key < CHAR_MAX && ((key >= 'a' && key <= 'z') || key == ';') && !isRepeat && GetKeyModifiers() == kModifier_None)
   {
      mHeldKeys += (char)key;
      mMenuMode = MenuMode::SingleLetter;
      UpdateDisplay();
   }

   if (IsShowing())
   {
      if (key == OF_KEY_DOWN || key == OF_KEY_UP || key == OF_KEY_LEFT || key == OF_KEY_RIGHT)
      {
         int oldIndex = mHighlightIndex;
         mHighlightIndex = ofClamp(mHighlightIndex + (key == OF_KEY_DOWN || key == OF_KEY_RIGHT ? 1 : -1), 0, (int)mElements.size() - 1);
         int change = mHighlightIndex - oldIndex;
         mScrollOffset -= change * kItemSpacing;
         UpdatePosition();
         MoveMouseToIndex(mHighlightIndex);
      }

      if (key == OF_KEY_RETURN)
      {
         if (mHighlightIndex < 0 || mHighlightIndex >= mElements.size())
            mHighlightIndex = 0;
         OnSelectItem(mHighlightIndex);
      }

      if (key >= 0 && key < CHAR_MAX && (key >= 'a' && key <= 'z') && mMenuMode != MenuMode::SingleLetter)
      {
         mSearchString += (char)key;
         mMenuMode = MenuMode::Search;
         UpdateDisplay();
         MoveMouseToIndex(0);
      }

      if (mMenuMode == MenuMode::Search && key == juce::KeyPress::backspaceKey)
      {
         if (mSearchString.length() > 0)
         {
            mSearchString = mSearchString.substring(0, mSearchString.length() - 1);
            if (mSearchString.length() == 0)
            {
               mMenuMode = MenuMode::ModuleCategories;
               UpdateDisplay();
            }
            else
            {
               UpdateDisplay();
               MoveMouseToIndex(0);
            }
         }
      }
   }

   if (key == OF_KEY_ESC)
      SetShowing(false);
}

void QuickSpawnMenu::KeyReleased(int key)
{
   if (mMenuMode == MenuMode::SingleLetter && key >= 0 && key < CHAR_MAX)
   {
      mHeldKeys = mHeldKeys.removeCharacters(juce::String::charToString((char)key));
      UpdateDisplay();
   }
}

void QuickSpawnMenu::UpdateDisplay()
{
   if ((mMenuMode == MenuMode::SingleLetter && mHeldKeys.isEmpty()) || TheSynth->GetMoveModule() != nullptr)
   {
      SetShowing(false);
   }
   else
   {
      if (mMenuMode == MenuMode::ModuleCategories)
      {
         mElements.clear();
         for (auto* dropdown : TheTitleBar->GetSpawnLists())
         {
            std::string label = dropdown->GetLabel();
            ofStringReplace(label, ":", "");
            ModuleFactory::Spawnable dummy;
            dummy.mLabel = label;
            mElements.push_back(dummy);
         }
      }
      else if (mMenuMode == MenuMode::SingleCategory)
      {
         mElements.clear();
         const auto& elements = TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->GetElements();
         for (size_t i = 0; i < elements.size(); ++i)
            mElements.push_back(elements[i]);
      }
      else if (mMenuMode == MenuMode::Search)
      {
         mElements = TheSynth->GetModuleFactory()->GetSpawnableModules(mSearchString.toStdString(), true);
      }
      else
      {
         mElements = TheSynth->GetModuleFactory()->GetSpawnableModules(mHeldKeys.toStdString(), false);
         mScrollOffset = 0;
      }

      float width = 150;
      for (auto& element : mElements)
      {
         float elementWidth = GetStringWidth(element.mLabel + " " + element.mDecorator) + 10 + (mMenuMode == MenuMode::SingleLetter ? 0 : kRightClickShiftX);
         if (elementWidth > width)
            width = elementWidth;
      }

      SetDimensions(width, MAX((int)mElements.size(), 1) * kItemSpacing);
      UpdatePosition();
      SetShowing(true);
   }
}

void QuickSpawnMenu::UpdatePosition()
{
   float minX = 5;
   float maxX = ofGetWidth() / GetOwningContainer()->GetDrawScale() - mWidth - 5;
   float minY = TheTitleBar->GetRect().height + 5;
   float maxY = ofGetHeight() / GetOwningContainer()->GetDrawScale() - mHeight - 5;

   if (mMenuMode == MenuMode::SingleLetter)
   {
      SetPosition(ofClamp(mAppearAtMousePos.x - mWidth / 2, minX, maxX),
                  ofClamp(mAppearAtMousePos.y - mHeight / 2, minY, maxY) + mScrollOffset);
   }
   else
   {
      SetPosition(ofClamp(mAppearAtMousePos.x - 5, minX, maxX),
                  mAppearAtMousePos.y - kItemSpacing / 2 + mScrollOffset);
   }
}

void QuickSpawnMenu::MouseReleased()
{
}

bool QuickSpawnMenu::MouseScrolled(float x, float y, float scrollX, float scrollY)
{
   float newY = ofClamp(y + scrollY * 5, kItemSpacing / 2, mHeight - kItemSpacing / 2);
   float changeAmount = newY - y;
   mScrollOffset -= changeAmount;
   UpdatePosition();

   return true;
}

void QuickSpawnMenu::MoveMouseToIndex(int index)
{
   mHighlightIndex = index;
   TheSynth->SetMousePosition(GetOwningContainer(), mX + 5, mY + (index + .5f) * kItemSpacing);
}

void QuickSpawnMenu::DrawModule()
{
   ofPushStyle();

   mHighlightIndex = -1;
   if (TheSynth->GetMouseY(GetOwningContainer()) > GetPosition().y)
      mHighlightIndex = GetIndexAt(TheSynth->GetMouseX(GetOwningContainer()) - GetPosition().x, TheSynth->GetMouseY(GetOwningContainer()) - GetPosition().y);

   ofSetColor(50, 50, 50, 100);
   ofFill();
   ofRect(-2, -2, mWidth + 4, mHeight + 4);
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mMenuMode == MenuMode::ModuleCategories)
         ofSetColor(IDrawableModule::GetColor(TheTitleBar->GetSpawnLists()[i]->GetCategory()) * (i == mHighlightIndex ? .7f : .5f), 255);
      else
         ofSetColor(IDrawableModule::GetColor(TheSynth->GetModuleFactory()->GetModuleType(mElements[i])) * (i == mHighlightIndex ? .7f : .5f), 255);
      ofRect(0, i * kItemSpacing + 1, mWidth, kItemSpacing - 1);
      if (i == mHighlightIndex)
         ofSetColor(255, 255, 0);
      else
         ofSetColor(255, 255, 255);

      bool showDecorator = true;
      if (mMenuMode == MenuMode::SingleCategory && mSelectedCategoryIndex >= 0 && mSelectedCategoryIndex < (int)TheTitleBar->GetSpawnLists().size())
         showDecorator = TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->ShouldShowDecorators();

      DrawTextNormal(mElements[i].mLabel + (showDecorator ? (" " + mElements[i].mDecorator) : ""), 1 + (mMenuMode == MenuMode::SingleLetter ? 0 : kRightClickShiftX), i * kItemSpacing + 12);
   }
   if (mElements.size() == 0)
   {
      ofSetColor(255, 255, 255);
      DrawTextNormal("no modules found", 1, 12);
   }

   ofPopStyle();
}

void QuickSpawnMenu::DrawModuleUnclipped()
{
   if (mMenuMode == MenuMode::SingleLetter)
      DrawTextBold(mHeldKeys.toStdString(), 3, -2, 17);
   if (mMenuMode == MenuMode::Search)
      DrawTextBold(mSearchString.toStdString(), 3, -2, 17);
}

bool QuickSpawnMenu::MouseMoved(float x, float y)
{
   mLastHoverX = x;
   mLastHoverY = y;
   return false;
}

void QuickSpawnMenu::OnClicked(float x, float y, bool right)
{
   if (right)
   {
      if (IsShowing())
         SetShowing(false);
      return;
   }

   OnSelectItem(GetIndexAt(x, y));
}

void QuickSpawnMenu::OnSelectItem(int index)
{
   if (mMenuMode == MenuMode::SingleLetter || mMenuMode == MenuMode::Search)
   {
      if (index >= 0 && index < mElements.size())
      {
         IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(mElements[index], TheSynth->GetMouseX(TheSynth->GetRootContainer()) + kModuleGrabOffset.x, TheSynth->GetMouseY(TheSynth->GetRootContainer()) + kModuleGrabOffset.y);
         TheSynth->SetMoveModule(module, kModuleGrabOffset.x, kModuleGrabOffset.y, true);
      }
      SetShowing(false);
   }

   if (mMenuMode == MenuMode::SingleCategory)
   {
      if (mSelectedCategoryIndex >= 0 &&
          mSelectedCategoryIndex < (int)TheTitleBar->GetSpawnLists().size() &&
          index >= 0 &&
          index < TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->GetList()->GetNumValues())
      {
         IDrawableModule* module = TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->Spawn(index);
         TheSynth->SetMoveModule(module, kModuleGrabOffset.x, kModuleGrabOffset.y, true);
      }
      SetShowing(false);
   }

   if (mMenuMode == MenuMode::ModuleCategories)
   {
      mSelectedCategoryIndex = index;
      if (mSelectedCategoryIndex >= 0 && mSelectedCategoryIndex < mElements.size())
      {
         mMenuMode = MenuMode::SingleCategory;
         ResetAppearPos();
         UpdateDisplay();
      }
      else
      {
         SetShowing(false);
      }
   }
}

std::string QuickSpawnMenu::GetHoveredModuleTypeName()
{
   auto* element = GetElementAt(mLastHoverX, mLastHoverY);
   if (element)
      return element->mLabel;
   else
      return "";
}

int QuickSpawnMenu::GetIndexAt(int x, int y) const
{
   if (x >= 0 && x < mWidth)
      return y / kItemSpacing;
   return -1;
}

const ModuleFactory::Spawnable* QuickSpawnMenu::GetElementAt(int x, int y) const
{
   int index = GetIndexAt(x, y);
   if (index >= 0 && index < mElements.size())
      return &mElements[index];

   return nullptr;
}
