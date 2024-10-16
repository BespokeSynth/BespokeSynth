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

bool QuickSpawnMenu::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   const float kScrollSpeed = 5;

   if (isInvertedScroll)
      scrollY *= -1;

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

   OnSelectItem(GetIndexAt(x, y));
}

void QuickSpawnMenu::OnSelectItem(int index)
{
   if (mMenuMode == MenuMode::SingleLetter || mMenuMode == MenuMode::Search || mMenuMode == MenuMode::SingleCategory)
   {
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
         mMenuMode = MenuMode::SingleCategory;
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
