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
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "UserPrefs.h"

#include "juce_gui_basics/juce_gui_basics.h"

QuickSpawnMenu* TheQuickSpawnMenu = nullptr;

namespace
{
   const int kItemSpacing = 15;
   const int kRightClickShiftX = 11;
   //touch input mode lays categories and modules out as a grid of buttons, sized like the title bar's menu button
   const float kGridCellHeight = 32;
   const float kGridCellMinWidth = 50;
   const float kGridSpacing = 3;
   const float kGridPageButtonWidth = 32;
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
   delete mMainContainerFollower;
}

void QuickSpawnMenu::Init()
{
   IDrawableModule::Init();
   SetShouldDrawOutline(false);
   mMainContainerFollower = new QuickSpawnFollower();
   mMainContainerFollower->SetName("quickspawnfollower");
   mMainContainerFollower->CreateUIControls();
   mMainContainerFollower->SetUp();
   mMainContainerFollower->SetShowing(false);
   Hide();
}

void QuickSpawnFollower::SetUp()
{
   mTempConnectionCable = new PatchCableSource(this, kConnectionType_Special);
   AddPatchCableSource(mTempConnectionCable);
   mTempConnectionCable->SetShowing(false);
}

void QuickSpawnMenu::ShowSpawnCategoriesPopup()
{
   ResetAppearPos();
   mMenuMode = MenuMode::ModuleCategories;
   mSearchString = "";
   mFilterForCable = nullptr;
   UpdateDisplay();
}

void QuickSpawnMenu::ShowSpawnCategoriesPopupAt(float x, float y)
{
   mAppearAtMousePos.set(x, y);
   mScrollOffset = 0;
   mGridFirstRow = 0;
   mMenuMode = MenuMode::ModuleCategories;
   mSearchString = "";
   mFilterForCable = nullptr;
   UpdateDisplay();
}

void QuickSpawnMenu::ShowSpawnCategoriesPopupForCable(PatchCable* cable)
{
   ResetAppearPos();
   mMenuMode = MenuMode::ModuleCategories;
   mSearchString = "";
   mFilterForCable = cable;
   mMainContainerFollower->mTempConnectionCable->SetShowing(false);
   UpdateDisplay();
}

void QuickSpawnMenu::SetTempConnection(IClickable* target, ConnectionType connectionType)
{
   mMainContainerFollower->mTempConnectionCable->SetConnectionType(connectionType);
   mMainContainerFollower->mTempConnectionCable->SetTarget(target);
   mMainContainerFollower->mTempConnectionCable->SetShowing(true);
}

void QuickSpawnMenu::ResetAppearPos()
{
   mAppearAtMousePos.set(TheSynth->GetMouseX(GetOwningContainer()), TheSynth->GetMouseY(GetOwningContainer()));
   mScrollOffset = 0;
   mGridFirstRow = 0;
}

void QuickSpawnMenu::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);

   if (!IsShowing())
      ResetAppearPos();

   if (!IsShowing() && PatchCable::sActivePatchCable != nullptr && key >= 'a' && key <= 'z')
      PatchCable::sActivePatchCable->ShowQuickspawnForCable();

   if ((!IsShowing() || mMenuMode == MenuMode::SingleLetter) && key >= 0 && key < CHAR_MAX && ((key >= 'a' && key <= 'z') || key == ';') && !isRepeat && GetKeyModifiers() == kModifier_None)
   {
      mHeldKeys += (char)key;
      mMenuMode = MenuMode::SingleLetter;
      mFilterForCable = nullptr;
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

      if (key >= 0 && key < CHAR_MAX && juce::CharacterFunctions::isPrintable((char)key) && mMenuMode != MenuMode::SingleLetter)
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
      Hide();
}

void QuickSpawnMenu::KeyReleased(int key)
{
   if (mMenuMode == MenuMode::SingleLetter && key >= 0 && key < CHAR_MAX)
   {
      mHeldKeys = mHeldKeys.removeCharacters(juce::String::charToString((char)key));
      UpdateDisplay();
   }
}

void QuickSpawnMenu::Hide()
{
   SetShowing(false);
   if (mFilterForCable != nullptr)
   {
      if (mMainContainerFollower->mTempConnectionCable->IsShowing())
         mFilterForCable->SetTempDrawTarget(nullptr);
      else
         mFilterForCable->GetOwner()->SetPatchCableTarget(mFilterForCable, nullptr, true);
   }
   mFilterForCable = nullptr;
   mMainContainerFollower->mTempConnectionCable->SetShowing(false);
}

void QuickSpawnMenu::UpdateDisplay()
{
   if ((mMenuMode == MenuMode::SingleLetter && mHeldKeys.isEmpty()) || TheSynth->GetMoveModule() != nullptr)
   {
      Hide();
   }
   else
   {
      if (mMenuMode == MenuMode::ModuleCategories)
      {
         mElements.clear();
         mCategoryIndices.clear();
         int categoryIndex = 0;
         for (auto* dropdown : TheTitleBar->GetSpawnLists())
         {
            bool containsValidTarget = false;
            if (mFilterForCable == nullptr)
            {
               containsValidTarget = true;
            }
            else
            {
               const auto& spawnables = dropdown->GetElements();
               for (size_t i = 0; i < spawnables.size(); ++i)
               {
                  if (MatchesFilter(spawnables[i]))
                     containsValidTarget = true;
               }
            }

            if (containsValidTarget)
            {
               std::string label = dropdown->GetLabel();
               ofStringReplace(label, ":", "");
               ModuleFactory::Spawnable dummy;
               dummy.mLabel = label;
               mElements.push_back(dummy);
               mCategoryIndices.push_back(categoryIndex);
            }

            ++categoryIndex;
         }
      }
      else if (mMenuMode == MenuMode::SingleCategory)
      {
         mElements.clear();
         const auto& elements = TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->GetElements();
         for (size_t i = 0; i < elements.size(); ++i)
         {
            if (MatchesFilter(elements[i]))
               mElements.push_back(elements[i]);
         }
      }
      else if (mMenuMode == MenuMode::Search)
      {
         mElements.clear();
         const auto& elements = TheSynth->GetModuleFactory()->GetSpawnableModules(mSearchString.toStdString(), true);
         int exactMatches = 0;
         for (size_t i = 0; i < elements.size(); ++i)
         {
            if (MatchesFilter(elements[i]))
            {
               if (juce::String(elements[i].mLabel).startsWith(mSearchString))
               {
                  mElements.insert(mElements.begin() + exactMatches, elements[i]);
                  ++exactMatches;
               }
               else
               {
                  mElements.push_back(elements[i]);
               }
            }
         }
      }
      else
      {
         mElements.clear();
         const auto& elements = TheSynth->GetModuleFactory()->GetSpawnableModules(mHeldKeys.toStdString(), false);
         for (size_t i = 0; i < elements.size(); ++i)
         {
            if (MatchesFilter(elements[i]))
               mElements.push_back(elements[i]);
         }
         mScrollOffset = 0;
      }

      if (IsGridLayout())
      {
         mGridCellWidth = kGridCellMinWidth;
         for (int i = 0; i < (int)mElements.size(); ++i)
            mGridCellWidth = MAX(mGridCellWidth, GetStringWidth(GetElementLabel(i)) + 10);

         const int numElements = MAX(1, (int)mElements.size());
         const float screenWidth = ofGetWidth() / GetOwningContainer()->GetDrawScale() - 10;
         const float screenHeight = ofGetHeight() / GetOwningContainer()->GetDrawScale() - (TheTitleBar->GetRect().height + 5) - 5;
         const int maxRows = MAX(1, (int)((screenHeight + kGridSpacing) / (kGridCellHeight + kGridSpacing)));
         const int maxColumns = MAX(1, (int)((screenWidth + kGridSpacing) / (mGridCellWidth + kGridSpacing)));

         //aim for a square grid, widened as needed to fit the screen's height, and narrowed to fit its width
         int columns = (int)ceil(sqrt((float)numElements));
         columns = MAX(columns, (numElements + maxRows - 1) / maxRows);
         columns = MIN(columns, maxColumns);
         int rows = (numElements + columns - 1) / columns;

         if (rows > maxRows)
         {
            //doesn't fit, so page through it with arrow buttons on the right, using every column that fits next to them
            int pagedMaxColumns = MAX(1, (int)((screenWidth - kGridPageButtonWidth + kGridSpacing) / (mGridCellWidth + kGridSpacing)));
            columns = MIN(numElements, pagedMaxColumns);
            rows = (numElements + columns - 1) / columns;
         }

         mGridColumns = MAX(1, columns);
         mGridRows = MAX(1, rows);
         mGridVisibleRows = MIN(mGridRows, maxRows);
         mGridFirstRow = ofClamp(mGridFirstRow, 0, mGridRows - mGridVisibleRows);

         float width = mGridColumns * mGridCellWidth + (mGridColumns - 1) * kGridSpacing;
         if (IsGridPaged())
            width += kGridSpacing + kGridPageButtonWidth;
         SetDimensions(width, mGridVisibleRows * kGridCellHeight + (mGridVisibleRows - 1) * kGridSpacing);
      }
      else
      {
         float width = 150;
         for (auto& element : mElements)
         {
            float elementWidth = GetStringWidth(element.mLabel + " " + element.mDecorator) + 10 + (mMenuMode == MenuMode::SingleLetter ? 0 : kRightClickShiftX);
            if (elementWidth > width)
               width = elementWidth;
         }

         SetDimensions(width, MAX((int)mElements.size(), 1) * kItemSpacing);
      }
      UpdatePosition();
      SetShowing(true);
   }
}

bool QuickSpawnMenu::MatchesFilter(const ModuleFactory::Spawnable& spawnable) const
{
   if (mFilterForCable == nullptr)
      return true;

   bool inputMatches = false;
   bool outputMatches = false;

   ModuleFactory::ModuleInfo info;

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Preset)
      info = TheSynth->GetModuleFactory()->GetModuleInfo(spawnable.mPresetModuleType);
   else
      info = TheSynth->GetModuleFactory()->GetModuleInfo(spawnable.mLabel);

   if (mFilterForCable->GetConnectionType() == kConnectionType_Note)
   {
      if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Plugin)
         inputMatches = true;
      if (info.mCanReceiveNotes)
         inputMatches = true;
   }

   if (mFilterForCable->GetConnectionType() == kConnectionType_Audio)
   {
      if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Plugin)
         inputMatches = true;
      if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::EffectChain)
         inputMatches = true;
      if (info.mCanReceiveAudio)
         inputMatches = true;
   }

   if (mFilterForCable->GetConnectionType() == kConnectionType_Pulse)
   {
      if (info.mCanReceivePulses)
         inputMatches = true;
   }

   if (mMainContainerFollower->mTempConnectionCable->IsShowing())
   {
      if (mMainContainerFollower->mTempConnectionCable->GetConnectionType() == kConnectionType_Note)
      {
         if (info.mCategory == kModuleCategory_Instrument)
            outputMatches = true;
         if (info.mCategory == kModuleCategory_Note)
            outputMatches = true;
      }

      if (mMainContainerFollower->mTempConnectionCable->GetConnectionType() == kConnectionType_Audio)
      {
         if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Plugin)
            outputMatches = true;
         if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::EffectChain)
            outputMatches = true;
         if (info.mCategory == kModuleCategory_Synth)
            outputMatches = true;
         if (info.mCategory == kModuleCategory_Audio)
            outputMatches = true;
         if (info.mCategory == kModuleCategory_Processor)
            outputMatches = true;
      }

      if (mMainContainerFollower->mTempConnectionCable->GetConnectionType() == kConnectionType_Pulse)
      {
         if (info.mCategory == kModuleCategory_Pulse)
            outputMatches = true;
      }
   }
   else
   {
      outputMatches = true; //doesn't matter
   }

   return inputMatches && outputMatches;
}

bool QuickSpawnMenu::IsGridLayout() const
{
   return UserPrefs.touch_input_mode.Get() && (mMenuMode == MenuMode::ModuleCategories || mMenuMode == MenuMode::SingleCategory);
}

ofRectangle QuickSpawnMenu::GetGridCellRect(int index) const
{
   int column = index % mGridColumns;
   int row = index / mGridColumns - mGridFirstRow;
   return ofRectangle(column * (mGridCellWidth + kGridSpacing), row * (kGridCellHeight + kGridSpacing), mGridCellWidth, kGridCellHeight);
}

std::string QuickSpawnMenu::GetElementLabel(int index) const
{
   bool showDecorator = true;
   if (mMenuMode == MenuMode::SingleCategory && mSelectedCategoryIndex >= 0 && mSelectedCategoryIndex < (int)TheTitleBar->GetSpawnLists().size())
      showDecorator = TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->ShouldShowDecorators();

   const auto& element = mElements[index];
   if (showDecorator && !element.mDecorator.empty())
      return element.mLabel + " " + element.mDecorator;
   return element.mLabel;
}

void QuickSpawnMenu::UpdatePosition()
{
   float minX = 5;
   float maxX = ofGetWidth() / GetOwningContainer()->GetDrawScale() - mWidth - 5;
   float minY = TheTitleBar->GetRect().height + 5;
   float maxY = ofGetHeight() / GetOwningContainer()->GetDrawScale() - mHeight - 5;

   if (IsGridLayout())
   {
      //keep it on screen if possible. if it's too tall to fit, keep the top visible
      float x = MAX(minX, MIN(mAppearAtMousePos.x, maxX));
      float y = MAX(minY, MIN(mAppearAtMousePos.y, maxY));
      SetPosition(x, y + mScrollOffset);
   }
   else if (mMenuMode == MenuMode::SingleLetter)
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

ofRectangle QuickSpawnMenu::GetGridPageButtonRect(bool up) const
{
   float x = mWidth - kGridPageButtonWidth;
   if (up)
      return ofRectangle(x, 0, kGridPageButtonWidth, kGridCellHeight);
   return ofRectangle(x, mHeight - kGridCellHeight, kGridPageButtonWidth, kGridCellHeight);
}

void QuickSpawnMenu::ChangeGridPage(int direction)
{
   mGridFirstRow = ofClamp(mGridFirstRow + direction * mGridVisibleRows, 0, mGridRows - mGridVisibleRows);
}

bool QuickSpawnMenu::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   const float kScrollSpeed = 5;

   if (isInvertedScroll)
      scrollY *= -1;

   if (IsGridLayout())
   {
      //scroll a row at a time
      if (!isSmoothScroll)
         scrollY = scrollY > 0 ? 1 : (scrollY < 0 ? -1 : 0);
      else
         scrollY /= 10;
      mGridScrollAccumulator += scrollY;
      while (fabsf(mGridScrollAccumulator) >= 1)
      {
         int direction = mGridScrollAccumulator > 0 ? -1 : 1;
         mGridFirstRow = ofClamp(mGridFirstRow + direction, 0, mGridRows - mGridVisibleRows);
         mGridScrollAccumulator += direction;
      }
      return true;
   }

   if (!isSmoothScroll)
   {
      if (scrollY < 0)
         scrollY = -kItemSpacing / kScrollSpeed;
      if (scrollY > 0)
         scrollY = kItemSpacing / kScrollSpeed;
   }

   float newY = ofClamp(y - scrollY * kScrollSpeed, kItemSpacing / 2, mHeight - kItemSpacing / 2);
   float changeAmount = newY - y;
   mScrollOffset -= changeAmount;
   UpdatePosition();

   return true;
}

void QuickSpawnMenu::MoveMouseToIndex(int index)
{
   mHighlightIndex = index;
   if (IsGridLayout())
   {
      int row = index / mGridColumns;
      if (row < mGridFirstRow)
         mGridFirstRow = row;
      if (row >= mGridFirstRow + mGridVisibleRows)
         mGridFirstRow = row - mGridVisibleRows + 1;
      ofVec2f center = GetGridCellRect(index).getCenter();
      TheSynth->SetMousePosition(GetOwningContainer(), mX + center.x, mY + center.y);
      return;
   }
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
   if (IsGridLayout())
   {
      for (int i = mGridFirstRow * mGridColumns; i < MIN((int)mElements.size(), (mGridFirstRow + mGridVisibleRows) * mGridColumns); ++i)
      {
         ModuleCategory category;
         if (mMenuMode == MenuMode::ModuleCategories)
            category = TheTitleBar->GetSpawnLists()[mCategoryIndices[i]]->GetCategory();
         else
            category = TheSynth->GetModuleFactory()->GetModuleCategory(mElements[i]);

         ofRectangle rect = GetGridCellRect(i);
         ofSetColor(IDrawableModule::GetColor(category) * (i == mHighlightIndex ? .7f : .5f), 255);
         ofFill();
         ofRect(rect, 4);
         ofSetColor(IDrawableModule::GetColor(category), 255);

         if (i == mHighlightIndex)
            ofSetColor(255, 255, 0);
         else
            ofSetColor(255, 255, 255);
         std::string label = GetElementLabel(i);
         DrawTextNormal(label, rect.x + (rect.width - GetStringWidth(label)) / 2, rect.y + rect.height / 2 + 4);
      }

      if (IsGridPaged())
      {
         float mouseX = TheSynth->GetMouseX(GetOwningContainer()) - GetPosition().x;
         float mouseY = TheSynth->GetMouseY(GetOwningContainer()) - GetPosition().y;
         for (bool up : { true, false })
         {
            ofRectangle rect = GetGridPageButtonRect(up);
            bool canPage = up ? mGridFirstRow > 0 : mGridFirstRow < mGridRows - mGridVisibleRows;
            bool hovered = canPage && rect.contains(mouseX, mouseY);
            ofSetColor(hovered ? ofColor(110, 110, 110) : ofColor(80, 80, 80), 255);
            ofFill();
            ofRect(rect, 4);
            ofSetColor(canPage ? ofColor(255, 255, 255) : ofColor(130, 130, 130), 255);
            ofVec2f c = rect.getCenter();
            const float kArrowSize = 7;
            if (up)
               ofTriangle(c.x - kArrowSize, c.y + kArrowSize / 2, c.x + kArrowSize, c.y + kArrowSize / 2, c.x, c.y - kArrowSize / 2);
            else
               ofTriangle(c.x - kArrowSize, c.y - kArrowSize / 2, c.x + kArrowSize, c.y - kArrowSize / 2, c.x, c.y + kArrowSize / 2);
         }

         //page indicator between the arrows
         std::string pageText = ofToString(mGridFirstRow / mGridVisibleRows + (mGridFirstRow % mGridVisibleRows != 0 ? 2 : 1)) + "/" + ofToString((mGridRows + mGridVisibleRows - 1) / mGridVisibleRows);
         ofRectangle upRect = GetGridPageButtonRect(true);
         ofRectangle downRect = GetGridPageButtonRect(false);
         if (downRect.y - (upRect.y + upRect.height) > 14)
         {
            ofSetColor(200, 200, 200);
            DrawTextNormal(pageText, upRect.getCenter().x - GetStringWidth(pageText) / 2, (upRect.y + upRect.height + downRect.y) / 2 + 4);
         }
      }
   }
   for (int i = 0; i < (int)mElements.size() && !IsGridLayout(); ++i)
   {
      if (mMenuMode == MenuMode::ModuleCategories)
         ofSetColor(IDrawableModule::GetColor(TheTitleBar->GetSpawnLists()[mCategoryIndices[i]]->GetCategory()) * (i == mHighlightIndex ? .7f : .5f), 255);
      else
         ofSetColor(IDrawableModule::GetColor(TheSynth->GetModuleFactory()->GetModuleCategory(mElements[i])) * (i == mHighlightIndex ? .7f : .5f), 255);
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

   mMainContainerFollower->UpdateLocation();
}

void QuickSpawnMenu::DrawModuleUnclipped()
{
   if (mMenuMode == MenuMode::SingleLetter)
      DrawTextBold(mHeldKeys.toStdString(), 3, -2, 15);
   if (mMenuMode == MenuMode::Search)
      DrawTextBold(mSearchString.toStdString(), 3, -2, 15);
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
         Hide();
      return;
   }

   if (IsGridLayout() && IsGridPaged())
   {
      if (GetGridPageButtonRect(true).contains(x, y))
      {
         ChangeGridPage(-1);
         return;
      }
      if (GetGridPageButtonRect(false).contains(x, y))
      {
         ChangeGridPage(1);
         return;
      }
      if (x > mWidth - kGridPageButtonWidth - kGridSpacing)
         return; //the gap around the arrow buttons shouldn't dismiss the menu
   }

   OnSelectItem(GetIndexAt(x, y));
}

void QuickSpawnMenu::OnSelectItem(int index)
{
   if (mMenuMode == MenuMode::SingleLetter || mMenuMode == MenuMode::Search || mMenuMode == MenuMode::SingleCategory)
   {
      if (index >= 0 && index < mElements.size() && TheTitleBar->IsManagePluginsItem(mElements[index]))
      {
         Hide();
         TheTitleBar->ManagePlugins();
         return;
      }

      if (index >= 0 && index < mElements.size())
      {
         IDrawableModule* module = TheSynth->SpawnModuleOnTheFly(mElements[index], TheSynth->GetMouseX(TheSynth->GetRootContainer()) + kModuleGrabOffset.x, TheSynth->GetMouseY(TheSynth->GetRootContainer()) + kModuleGrabOffset.y);
         TheSynth->SetMoveModule(module, kModuleGrabOffset.x, kModuleGrabOffset.y, true);
         if (mFilterForCable != nullptr)
         {
            mFilterForCable->SetTempDrawTarget(nullptr);
            if (mMainContainerFollower->mTempConnectionCable->IsShowing())
               module->SetTarget(mMainContainerFollower->mTempConnectionCable->GetTarget());
            mFilterForCable->GetOwner()->SetPatchCableTarget(mFilterForCable, module, true);
            mFilterForCable = nullptr;
         }
      }
      Hide();
   }

   if (mMenuMode == MenuMode::ModuleCategories)
   {
      if (index >= 0 && index < mElements.size())
      {
         mSelectedCategoryIndex = mCategoryIndices[index];
         TheTitleBar->RefreshSpawnList(TheTitleBar->GetSpawnLists()[mSelectedCategoryIndex]->GetList());
         mMenuMode = MenuMode::SingleCategory;
         if (IsGridLayout())
         {
            mScrollOffset = 0; //the module grid replaces the category grid in place
            mGridFirstRow = 0;
         }
         else
            ResetAppearPos();
         UpdateDisplay();
      }
      else
      {
         Hide();
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
   if (IsGridLayout())
   {
      if (x < 0 || y < 0)
         return -1;
      int column = (int)(x / (mGridCellWidth + kGridSpacing));
      int row = (int)(y / (kGridCellHeight + kGridSpacing));
      if (column >= mGridColumns || row >= mGridVisibleRows)
         return -1;
      row += mGridFirstRow;
      int index = row * mGridColumns + column;
      if (index >= (int)mElements.size() || !GetGridCellRect(index).contains(x, y))
         return -1;
      return index;
   }

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

void QuickSpawnFollower::GetDimensions(float& width, float& height)
{
   TheQuickSpawnMenu->GetDimensions(width, height);
   float scaleFactor = UserPrefs.ui_scale.Get() / gDrawScale;
   width *= scaleFactor;
   height *= scaleFactor;

   height -= 10;
   width -= 10;
}

void QuickSpawnFollower::UpdateLocation()
{
   float x, y;
   TheQuickSpawnMenu->GetPosition(x, y);
   float scaleFactor = UserPrefs.ui_scale.Get() / gDrawScale;
   x *= scaleFactor;
   y *= scaleFactor;
   x -= TheSynth->GetDrawOffset().x;
   y -= TheSynth->GetDrawOffset().y;
   SetPosition(x, y);
}
