/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2024 Ryan Challinor (contact: awwbees@gmail.com)

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

    ControlInterface.cpp
    Created: 6 May 2024
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ControlInterface.h"

#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "MathUtils.h"
#include "UIControlMacros.h"

namespace
{
   const float kEditorWidth = 215;
   const float kEditorRowHeight = 18;
   const float kEditorColumn1 = 45;
   const float kEditorColumn2 = 147;
   const float kDragThreshold = 5;
   const int kMinControlSize = 5;
   const int kMaxControlSize = 2000;
}

ControlInterface::ControlInterface()
: IDrawableModule(130, 20)
{
}

void ControlInterface::Init()
{
   IDrawableModule::Init();
}

void ControlInterface::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mControlEditorBox = new CodeEntry(this, "control_editor", 3, 100, 300, 300);

   mAddControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mAddControlCable);

   mAutoPositionCheckbox = CreateEditorControl(new Checkbox(this, "auto-position", 5, 0, &mAutoPosition));
   mEditNameEntry = CreateEditorControl(new TextEntry(this, "edit name", kEditorColumn1, 0, 18, &mEditName));
   mEditMinEntry = CreateEditorControl(new TextEntry(this, "edit min", kEditorColumn1, 0, 7, &mEditMin, -FLT_MAX, FLT_MAX));
   mEditMaxEntry = CreateEditorControl(new TextEntry(this, "edit max", kEditorColumn2, 0, 7, &mEditMax, -FLT_MAX, FLT_MAX));
   mEditWidthEntry = CreateEditorControl(new TextEntry(this, "edit width", kEditorColumn1, 0, 7, &mEditWidth, kMinControlSize, kMaxControlSize));
   mEditHeightEntry = CreateEditorControl(new TextEntry(this, "edit height", kEditorColumn2, 0, 7, &mEditHeight, kMinControlSize, kMaxControlSize));
   mMoveUpButton = CreateEditorControl(new ClickButton(this, "move up", kEditorColumn1, 0));
   mMoveDownButton = CreateEditorControl(new ClickButton(this, "move down", kEditorColumn1 + 55, 0));
}

template <class T>
T* ControlInterface::CreateEditorControl(T* control)
{
   //take this out of the module's list of controls, so that it doesn't get treated as a part of the interface (control surfaces, snapshots, randomization, patching, name lookups...)
   RemoveUIControl(control, false);
   control->SetCableTargetable(false);
   control->SetShouldSaveState(false);
   control->SetIsRandomizable(false);
   control->SetShowing(false);
   mEditorControls.push_back(control);
   return control;
}

ControlInterface::~ControlInterface()
{
   for (auto* control : mEditorControls)
      control->Delete();
}

void ControlInterface::Poll()
{
   IDrawableModule::Poll();

   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr)
         control->mUIControl->SetNoHover(mEditMode); //so that clicks go to the module rather than directly to the control
   }

   if (mAutoPosition)
      AutoPositionControls();

   for (auto* control : mEditorControls)
      control->Poll();

   UpdateEditorFields();
}

void ControlInterface::SetEditMode(bool editMode)
{
   mEditMode = editMode;
   mDragControl = nullptr;
   SetSelectedControl(nullptr);

   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr)
      {
         control->mUIControl->SetNoHover(mEditMode);
         if (mEditMode && gHoveredUIControl == control->mUIControl)
            gHoveredUIControl = nullptr;
      }
   }
}

void ControlInterface::SetSelectedControl(ControlElement* control)
{
   if (control != mSelectedControl)
   {
      //don't let an in-progress text edit get applied to a different control
      for (auto* editorControl : mEditorControls)
      {
         TextEntry* entry = dynamic_cast<TextEntry*>(editorControl);
         if (entry != nullptr && IKeyboardFocusListener::GetActiveKeyboardFocus() == entry)
            IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
      }
   }

   mSelectedControl = control;
   UpdateEditorFields();
}

void ControlInterface::OnControlRemoved(ControlElement* control)
{
   if (mSelectedControl == control)
      SetSelectedControl(nullptr);
   if (mDragControl == control)
      mDragControl = nullptr;
   if (mCurrentEditControl == control)
      mCurrentEditControl = nullptr;
}

void ControlInterface::MoveSelectedControl(int direction)
{
   auto iter = std::find(mControls.begin(), mControls.end(), mSelectedControl);
   if (iter == mControls.end())
      return;
   if (direction < 0 && iter == mControls.begin())
      return;
   if (direction > 0 && std::next(iter) == mControls.end())
      return;

   auto other = direction < 0 ? std::prev(iter) : std::next(iter);
   SwapPatchCableSources((*iter)->mTargetCable, (*other)->mTargetCable); //cables are saved by index, and get recreated in the order of our controls when loading
   std::iter_swap(iter, other);
}

void ControlInterface::AutoPositionControls()
{
   float y = 3;
   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr)
      {
         control->SetPosition(5, y);
         y += control->mUIControl->GetRect(K(local)).height + 3;
      }
   }
}

void ControlInterface::UpdateEditorFields()
{
   if (mSelectedControl == nullptr || mSelectedControl->mUIControl == nullptr)
      return;

   mEditName = mSelectedControl->mUIControl->GetDisplayName();
   if (mSelectedControl->mFloatSlider != nullptr)
   {
      mEditMin = mSelectedControl->mFloatSlider->GetMin();
      mEditMax = mSelectedControl->mFloatSlider->GetMax();
   }
   if (mSelectedControl->mIntSlider != nullptr)
   {
      mEditMin = mSelectedControl->mIntSlider->GetMin();
      mEditMax = mSelectedControl->mIntSlider->GetMax();
   }
   ofRectangle rect = mSelectedControl->mUIControl->GetRect(K(local));
   mEditWidth = (int)rect.width;
   mEditHeight = (int)rect.height;
}

void ControlInterface::TextEntryComplete(TextEntry* entry)
{
   if (mSelectedControl == nullptr || mSelectedControl->mUIControl == nullptr)
      return;

   mSelectedControl->UpdateInfoFromControl(); //pick up any changes that were made directly to the control

   if (entry == mEditNameEntry)
      mSelectedControl->mInfo["display_name"] = mEditName;

   if ((entry == mEditMinEntry || entry == mEditMaxEntry) && mSelectedControl->IsSlider())
   {
      bool isInt = mSelectedControl->mIntSlider != nullptr;
      bool isValid = isInt ? ((int)mEditMin < (int)mEditMax) : (mEditMin < mEditMax);
      if (isValid)
      {
         const char* field = (entry == mEditMinEntry) ? "min" : "max";
         float value = (entry == mEditMinEntry) ? mEditMin : mEditMax;
         if (isInt)
            mSelectedControl->mInfo[field] = (int)value;
         else
            mSelectedControl->mInfo[field] = value;
      }
   }

   //only store a size if it was changed, so that controls that size themselves automatically continue to do so
   ofRectangle rect = mSelectedControl->mUIControl->GetRect(K(local));
   if (entry == mEditWidthEntry && mEditWidth != (int)rect.width)
      mSelectedControl->mInfo["width"] = mEditWidth;
   if (entry == mEditHeightEntry && mEditHeight != (int)rect.height)
      mSelectedControl->mInfo["height"] = mEditHeight;

   mSelectedControl->ApplyInfoToControl();
   UpdateEditorFields();
}

void ControlInterface::GetPush2OverrideControls(std::vector<IUIControl*>& controls) const
{
   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr && control->mUIControl->IsShowing())
         controls.push_back(control->mUIControl);
   }
}

ControlInterface::ControlElement* ControlInterface::GetControlAt(float x, float y) const
{
   for (auto iter = mControls.rbegin(); iter != mControls.rend(); ++iter) //backwards, to prefer what was drawn on top
   {
      if ((*iter)->mUIControl != nullptr && (*iter)->mUIControl->GetRect(K(local)).contains(x, y))
         return *iter;
   }
   return nullptr;
}

void ControlInterface::OnClicked(float x, float y, bool right)
{
   if (mEditMode && !Minimized() && y >= 0)
   {
      for (auto* control : mEditorControls)
      {
         if (control->TestClick(x, y, right))
            return;
      }

      if (y < mEditorY)
      {
         if (right)
            return;

         mDragControl = GetControlAt(x, y);
         mHasDragged = false;
         if (mDragControl != nullptr)
         {
            mDragStartMousePos.set(x, y);
            mDragStartControlPos = mDragControl->mUIControl->GetPosition(K(local));
         }
         else
         {
            SetSelectedControl(nullptr);
         }
         return;
      }
   }

   IDrawableModule::OnClicked(x, y, right);
}

bool ControlInterface::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (Minimized() || !mEditMode)
      return false;

   if (mDragControl != nullptr)
   {
      ofVec2f delta = ofVec2f(x, y) - mDragStartMousePos;
      if (delta.lengthSquared() > kDragThreshold * kDragThreshold)
         mHasDragged = true;
      if (mHasDragged && !mAutoPosition)
         mDragControl->SetPosition(MAX(0.0f, mDragStartControlPos.x + delta.x), MAX(0.0f, mDragStartControlPos.y + delta.y));
   }
   else
   {
      for (auto* control : mEditorControls)
         control->NotifyMouseMoved(x, y);
   }

   return false;
}

void ControlInterface::MouseReleased()
{
   IDrawableModule::MouseReleased();

   for (auto* control : mEditorControls)
      control->MouseReleased();

   if (mDragControl != nullptr)
   {
      if (!mHasDragged) //it was a tap
         SetSelectedControl(mDragControl);
      mDragControl = nullptr;
   }
}

void ControlInterface::DrawEditor()
{
   bool hasSelection = mEditMode && mSelectedControl != nullptr && mSelectedControl->mUIControl != nullptr;
   bool hasSlider = hasSelection && mSelectedControl->IsSlider();

   mAutoPositionCheckbox->SetShowing(mEditMode);
   mEditNameEntry->SetShowing(hasSelection);
   mEditMinEntry->SetShowing(hasSlider);
   mEditMaxEntry->SetShowing(hasSlider);
   mEditWidthEntry->SetShowing(hasSelection);
   mEditHeightEntry->SetShowing(hasSelection);
   mMoveUpButton->SetShowing(hasSelection);
   mMoveDownButton->SetShowing(hasSelection);

   //only link entries that are visible, for tabbing between them
   mEditNameEntry->SetNextTextEntry(hasSlider ? mEditMinEntry : mEditWidthEntry);
   mEditMinEntry->SetNextTextEntry(mEditMaxEntry);
   mEditMaxEntry->SetNextTextEntry(hasSlider ? mEditWidthEntry : nullptr);
   mEditWidthEntry->SetNextTextEntry(mEditHeightEntry);

   if (!mEditMode)
      return;

   ofPushStyle();
   ofNoFill();
   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr)
      {
         bool highlighted = control == mSelectedControl || control == mDragControl;
         ofSetColor(255, 255, 0, highlighted ? 255 : 80);
         ofSetLineWidth(highlighted ? 1.5f : .5f);
         ofRect(control->mUIControl->GetRect(K(local)));
      }
   }

   mEditorY = mHeight;
   ofSetColor(255, 255, 255, gModuleDrawAlpha * .3f);
   ofSetLineWidth(.5f);
   mWidth = MAX(mWidth, kEditorWidth);
   ofLine(3, mEditorY, mWidth - 3, mEditorY);

   float y = mEditorY + 3;
   ofSetColor(255, 255, 255, gModuleDrawAlpha);
   mAutoPositionCheckbox->SetPosition(5, y);
   y += kEditorRowHeight;

   if (hasSelection)
   {
      DrawTextRightJustify("name", kEditorColumn1 - 3, y + 12);
      mEditNameEntry->SetPosition(kEditorColumn1, y);
      y += kEditorRowHeight;

      if (hasSlider)
      {
         DrawTextRightJustify("min", kEditorColumn1 - 3, y + 12);
         mEditMinEntry->SetPosition(kEditorColumn1, y);
         DrawTextRightJustify("max", kEditorColumn2 - 3, y + 12);
         mEditMaxEntry->SetPosition(kEditorColumn2, y);
         y += kEditorRowHeight;
      }

      DrawTextRightJustify("width", kEditorColumn1 - 3, y + 12);
      mEditWidthEntry->SetPosition(kEditorColumn1, y);
      DrawTextRightJustify("height", kEditorColumn2 - 3, y + 12);
      mEditHeightEntry->SetPosition(kEditorColumn2, y);
      y += kEditorRowHeight;

      int index = (int)std::distance(mControls.begin(), std::find(mControls.begin(), mControls.end(), mSelectedControl));
      DrawTextRightJustify(ofToString(index + 1) + "/" + ofToString((int)mControls.size()), kEditorColumn1 - 3, y + 12);
      mMoveUpButton->SetPosition(kEditorColumn1, y);
      mMoveDownButton->PositionTo(mMoveUpButton, kAnchor_Right_Padded);
      y += kEditorRowHeight;
   }

   mHeight = y + 2;
   ofPopStyle();

   for (auto* control : mEditorControls)
      control->Draw();
}

void ControlInterface::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (auto* control : mControls)
   {
      if (control->mUIControl != nullptr)
         control->mUIControl->Draw();
   }

   mAddControlCable->SetShowing(mShowCables);

   mWidth = 90;
   mHeight = 20;
   for (auto* control : mControls)
   {
      control->mTargetCable->SetShowing(mShowCables);
      if (control != nullptr && control->mUIControl != nullptr)
      {
         ofRectangle rect = control->mUIControl->GetRect(K(local));
         mWidth = MAX(mWidth, rect.x + rect.width + 3);
         mHeight = MAX(mHeight, rect.y + rect.height + 3);

         control->mTargetCable->SetManualPosition(rect.getMaxX() + 5, rect.getCenter().y);
      }
   }

   DrawEditor();

   mControlEditorBox->SetShowing(mCurrentEditControl != nullptr);
   if (mCurrentEditControl != nullptr)
   {
      mControlEditorBox->SetPosition(3, mHeight);
      ofRectangle rect = mControlEditorBox->GetRect(K(local));
      mWidth = MAX(mWidth, rect.x + rect.width + 3);
      mHeight = rect.y + rect.height + 3;

      mControlEditorBox->Draw();

      if (mControlEditorBox != IKeyboardFocusListener::GetActiveKeyboardFocus())
         mCurrentEditControl = nullptr;
   }
}

void ControlInterface::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);

   if (key == OF_KEY_RETURN && GetKeyModifiers() == kModifier_Shift)
   {
      for (auto* control : mControls)
      {
         if (control != nullptr && control->mUIControl != nullptr && control->mUIControl == gHoveredUIControl)
         {
            mControlEditorBox->SetText(control->mInfo.getRawString(K(pretty)));
            mControlEditorBox->ResetScroll();
            mCurrentEditControl = control;
            IKeyboardFocusListener::SetActiveKeyboardFocus(mControlEditorBox);
         }
      }
   }
}

void ControlInterface::ExecuteCode()
{
   if (mCurrentEditControl != nullptr)
   {
      mCurrentEditControl->mInfo.parse(mControlEditorBox->GetText(true));
      mCurrentEditControl->ApplyInfoToControl();
   }
}

ofVec2f ControlInterface::FindNewUIControlPos()
{
   float x = 5;
   float y = 3;
   bool hasCollision = false;
   do
   {
      hasCollision = false;
      for (auto* otherControl : mControls)
      {
         if (otherControl != nullptr && otherControl->mUIControl != nullptr)
         {
            ofRectangle rect = otherControl->mUIControl->GetRect(K(local));
            if (rect.intersects(ofRectangle(5, y, 80, 15)))
            {
               hasCollision = true;
               y += 18;
               break;
            }
         }
      }
   } while (hasCollision);

   return ofVec2f(x, y);
}

void ControlInterface::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (fromUserClick)
   {
      bool repatched = false;
      auto insertPos = mControls.end();
      ofVec2f controlPos = FindNewUIControlPos();
      IUIControl* targetUIControl = dynamic_cast<IUIControl*>(cableSource->GetTarget());

      for (auto iter = mControls.begin(); iter != mControls.end(); ++iter)
      {
         ControlElement* control = *iter;
         if (control->GetCableSource() == cableSource)
         {
            if (targetUIControl != nullptr)
               repatched = true;

            //if we're repatching, the new control takes the old one's position and place in the order
            if (control->mUIControl != nullptr)
               controlPos = control->mUIControl->GetPosition(K(local));
            OnControlRemoved(control);
            RemovePatchCableSource(cableSource);
            if (control->mUIControl != nullptr)
               RemoveUIControl(control->mUIControl);
            insertPos = mControls.erase(iter);
            break;
         }
      }

      if (cableSource == mAddControlCable || repatched)
      {
         if (targetUIControl != nullptr)
         {
            mAddControlCable->Clear();

            ControlElement* control = CreateControl(insertPos);

            control->mInfo["path"] = targetUIControl->Path(false, false, GetParent());
            control->mInfo["display_name"] = targetUIControl->GetDisplayName();
            control->mInfo["x"] = controlPos.x;
            control->mInfo["y"] = controlPos.y;

            FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(targetUIControl);
            if (floatSlider)
            {
               control->mInfo["type"] = "floatslider";
               control->mInfo["min"] = floatSlider->GetMin();
               control->mInfo["max"] = floatSlider->GetMax();
               control->mInfo["width"] = kDefaultSliderWidth;
               control->mInfo["default_value"] = floatSlider->GetDefaultValue();
            }

            IntSlider* intSlider = dynamic_cast<IntSlider*>(targetUIControl);
            if (intSlider)
            {
               control->mInfo["type"] = "intslider";
               control->mInfo["min"] = intSlider->GetMin();
               control->mInfo["max"] = intSlider->GetMax();
               control->mInfo["width"] = kDefaultSliderWidth;
               control->mInfo["default_value"] = intSlider->GetDefaultValue();
            }

            Checkbox* checkbox = dynamic_cast<Checkbox*>(targetUIControl);
            if (checkbox)
            {
               control->mInfo["type"] = "checkbox";
            }

            ClickButton* button = dynamic_cast<ClickButton*>(targetUIControl);
            if (button)
            {
               control->mInfo["type"] = "button";
            }

            DropdownList* dropdown = dynamic_cast<DropdownList*>(targetUIControl);
            if (dropdown)
            {
               control->mInfo["type"] = "dropdown";
               control->mInfo["values"].clear();
               for (int i = 0; i < dropdown->GetNumValues(); ++i)
               {
                  const auto& element = dropdown->GetElement(i);
                  ofxJSONElement json;
                  json["label"] = element.mLabel;
                  json["value"] = element.mValue;
                  control->mInfo["values"].append(json);
               }
            }

            RadioButton* radioButton = dynamic_cast<RadioButton*>(targetUIControl);
            if (radioButton)
            {
               control->mInfo["type"] = "radiobutton";
               control->mInfo["values"].clear();
               for (int i = 0; i < radioButton->GetNumValues(); ++i)
               {
                  const auto& element = radioButton->GetElement(i);
                  ofxJSONElement json;
                  json["label"] = element.mLabel;
                  json["value"] = element.mValue;
                  control->mInfo["values"].append(json);
               }
            }

            control->SetUpControl();
         }
         else
         {
         }
      }
   }
}

ControlInterface::ControlElement* ControlInterface::CreateControl(std::list<ControlElement*>::iterator insertPos)
{
   ControlElement* control = new ControlElement();
   auto newIter = mControls.insert(insertPos, control);
   control->Init(this);
   //the new cable source was added at the end, move it back so that cable order matches control order
   for (auto iter = std::prev(mControls.end()); iter != newIter; --iter)
      SwapPatchCableSources(control->mTargetCable, (*iter)->mTargetCable);
   return control;
}

ControlInterface::ControlElement* ControlInterface::FindOrCreateNewControl(std::string identifier, std::string type, bool& isNewControl)
{
   ControlElement* control = nullptr;
   auto insertPos = mControls.end();
   bool isReplacement = false;
   ofVec2f replacedPos;
   for (auto iter = mControls.begin(); iter != mControls.end(); ++iter)
   {
      ControlElement* existingControl = *iter;
      if (existingControl->mInfo["identifier"] == identifier)
      {
         if (existingControl->mInfo["type"] == type)
         {
            //we already have a control with this name, so just update the settings
            control = existingControl;
            isNewControl = false;
         }
         else
         {
            //we already have a control with this name but it's a different type, delete it and make a new one in its place
            isReplacement = true;
            if (existingControl->mUIControl != nullptr)
               replacedPos = existingControl->mUIControl->GetPosition(K(local));
            else
               replacedPos.set(existingControl->GetInfo("x", 0), existingControl->GetInfo("y", 0));
            OnControlRemoved(existingControl);
            RemovePatchCableSource(existingControl->mTargetCable);
            if (existingControl->mUIControl != nullptr)
               RemoveUIControl(existingControl->mUIControl);
            insertPos = mControls.erase(iter);
         }
         break;
      }
   }

   if (control == nullptr)
   {
      control = CreateControl(insertPos);
      control->mInfo["type"] = type;
      if (isReplacement)
      {
         control->mInfo["x"] = replacedPos.x;
         control->mInfo["y"] = replacedPos.y;
      }
      isNewControl = true;
   }

   return control;
}

FloatSlider* ControlInterface::AddFloatSlider(std::string name, float defaultVal, float min, float max)
{
   ofVec2f controlPos = FindNewUIControlPos();
   bool isNewControl = false;
   ControlElement* control = FindOrCreateNewControl(name, "floatslider", isNewControl);
   if (isNewControl)
      control->mDummyFloat = defaultVal;
   else
      controlPos = control->mUIControl->GetPosition(K(local));
   if (isNewControl && control->HasInfo("x")) //we replaced an existing control, take its position
      controlPos.set(control->GetInfo("x", 0), control->GetInfo("y", 0));

   control->mInfo["path"] = "";
   control->mInfo["display_name"] = name;
   control->mInfo["identifier"] = name;
   control->mInfo["x"] = controlPos.x;
   control->mInfo["y"] = controlPos.y;
   control->mInfo["min"] = min;
   control->mInfo["max"] = max;
   if (!control->HasInfo("width")) //don't stomp a width that was set in edit mode
      control->mInfo["width"] = kDefaultSliderWidth;
   control->mInfo["default_value"] = defaultVal;

   control->SetUpControl();

   return control->mFloatSlider;
}

IntSlider* ControlInterface::AddIntSlider(std::string name, int defaultVal, int min, int max)
{
   ofVec2f controlPos = FindNewUIControlPos();
   bool isNewControl = false;
   ControlElement* control = FindOrCreateNewControl(name, "intslider", isNewControl);
   if (isNewControl)
      control->mDummyInt = defaultVal;
   else
      controlPos = control->mUIControl->GetPosition(K(local));
   if (isNewControl && control->HasInfo("x")) //we replaced an existing control, take its position
      controlPos.set(control->GetInfo("x", 0), control->GetInfo("y", 0));

   control->mInfo["path"] = "";
   control->mInfo["display_name"] = name;
   control->mInfo["identifier"] = name;
   control->mInfo["x"] = controlPos.x;
   control->mInfo["y"] = controlPos.y;
   control->mInfo["min"] = min;
   control->mInfo["max"] = max;
   if (!control->HasInfo("width")) //don't stomp a width that was set in edit mode
      control->mInfo["width"] = kDefaultSliderWidth;
   control->mInfo["default_value"] = defaultVal;

   control->SetUpControl();

   return control->mIntSlider;
}

void ControlInterface::ClearAllControls()
{
   for (auto* control : mControls)
   {
      OnControlRemoved(control);
      RemovePatchCableSource(control->mTargetCable);
      if (control->mUIControl != nullptr)
         RemoveUIControl(control->mUIControl);
   }
   mControls.clear();
}

void ControlInterface::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   for (auto* control : mControls)
   {
      if (slider == control->mFloatSlider)
      {
         FloatSlider* attachedToFloatSlider = dynamic_cast<FloatSlider*>(control->mAttachedToUIControl);
         if (attachedToFloatSlider)
            attachedToFloatSlider->GetOwner()->FloatSliderUpdated(attachedToFloatSlider, oldVal, time);
      }
   }
}

void ControlInterface::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   for (auto* control : mControls)
   {
      if (slider == control->mIntSlider)
      {
         IntSlider* attachedToIntSlider = dynamic_cast<IntSlider*>(control->mAttachedToUIControl);
         if (attachedToIntSlider)
            attachedToIntSlider->GetOwner()->IntSliderUpdated(attachedToIntSlider, oldVal, time);
      }
   }
}

void ControlInterface::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (auto* control : mControls)
   {
      if (checkbox == control->mCheckbox)
      {
         Checkbox* attachedToCheckbox = dynamic_cast<Checkbox*>(control->mAttachedToUIControl);
         if (attachedToCheckbox)
            attachedToCheckbox->GetOwner()->CheckboxUpdated(attachedToCheckbox, time);
      }
   }
}

void ControlInterface::ButtonClicked(ClickButton* button, double time)
{
   if (button == mMoveUpButton)
      MoveSelectedControl(-1);
   if (button == mMoveDownButton)
      MoveSelectedControl(1);

   for (auto* control : mControls)
   {
      if (button == control->mButton)
      {
         ClickButton* attachedToButton = dynamic_cast<ClickButton*>(control->mAttachedToUIControl);
         if (attachedToButton)
            attachedToButton->GetOwner()->ButtonClicked(attachedToButton, time);
      }
   }
}

void ControlInterface::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   for (auto* control : mControls)
   {
      if (list == control->mDropdown)
      {
         DropdownList* attachedToDropdown = dynamic_cast<DropdownList*>(control->mAttachedToUIControl);
         if (attachedToDropdown)
            attachedToDropdown->GetOwner()->DropdownUpdated(attachedToDropdown, oldVal, time);
      }
   }
}

void ControlInterface::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
   for (auto* control : mControls)
   {
      if (radio == control->mRadioButton)
      {
         RadioButton* attachedToRadioButton = dynamic_cast<RadioButton*>(control->mAttachedToUIControl);
         if (attachedToRadioButton)
            attachedToRadioButton->GetOwner()->RadioButtonUpdated(attachedToRadioButton, oldVal, time);
      }
   }
}

void ControlInterface::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["auto_position"] = mAutoPosition;

   for (auto* control : mControls)
   {
      ofxJSONElement controlInfo;
      std::string controlType = "";
      if (control->mFloatSlider != nullptr)
         controlType = "floatslider";
      if (control->mIntSlider != nullptr)
         controlType = "intslider";
      if (control->mCheckbox != nullptr)
         controlType = "checkbox";
      if (control->mButton != nullptr)
         controlType = "button";
      if (control->mDropdown != nullptr)
         controlType = "dropdown";
      if (control->mRadioButton != nullptr)
         controlType = "radiobutton";

      controlInfo["type"] = controlType;
      controlInfo["identifier"] = control->mUIControl != nullptr ? (control->mUIControl->Name()) : "";
      moduleInfo["controls"].append(controlInfo);
   }
}

void ControlInterface::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("show_cables", moduleInfo, true);
   mModuleSaveData.SetBool("edit_mode", false); //always start out of edit mode
   if (!moduleInfo["auto_position"].isNull())
      mAutoPosition = moduleInfo["auto_position"].asBool();
   else
      mAutoPosition = moduleInfo["controls"].size() == 0; //on by default, but don't rearrange layouts that were saved before this option existed

   int numControls = moduleInfo["controls"].size();
   for (int i = 0; i < numControls; ++i)
   {
      ControlElement* control = new ControlElement();
      mControls.push_back(control);
      control->Init(this);

      std::string controlType = moduleInfo["controls"][i]["type"].asString();
      const char* identifier = moduleInfo["controls"][i]["identifier"].asCString();
      if (controlType == "floatslider")
         control->mFloatSlider = new FloatSlider(this, identifier, -1, -1, 80, 15, &control->mDummyFloat, 0, 1);
      if (controlType == "intslider")
         control->mIntSlider = new IntSlider(this, identifier, -1, -1, 80, 15, &control->mDummyInt, 0, 1);
      if (controlType == "checkbox")
         control->mCheckbox = new Checkbox(this, identifier, -1, -1, &control->mDummyBool);
      if (controlType == "button")
         control->mButton = new ClickButton(this, identifier, -1, -1);
      if (controlType == "dropdown")
         control->mDropdown = new DropdownList(this, identifier, -1, -1, &control->mDummyInt);
      if (controlType == "radiobutton")
         control->mRadioButton = new RadioButton(this, identifier, -1, -1, &control->mDummyInt);
   }

   SetUpFromSaveData();
}

void ControlInterface::SetUpFromSaveData()
{
   mShowCables = mModuleSaveData.GetBool("show_cables");
   if (mModuleSaveData.GetBool("edit_mode") != mEditMode)
      SetEditMode(!mEditMode);
}

void ControlInterface::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mControls.size();
   for (auto* control : mControls)
      control->SaveState(out);
}

void ControlInterface::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int numControls;
   in >> numControls;
   assert(numControls == (int)mControls.size());
   for (auto* control : mControls)
      control->LoadState(in, rev);
}

void ControlInterface::ControlElement::Init(ControlInterface* owner)
{
   mTargetCable = new PatchCableSource(owner, kConnectionType_Modulator);
   mTargetCable->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   mTargetCable->SetAllowMultipleTargets(false);
   owner->AddPatchCableSource(mTargetCable);
   mOwner = owner;
}

ControlInterface::ControlElement::~ControlElement()
{
   mOwner->RemovePatchCableSource(mTargetCable);
   if (mUIControl != nullptr)
      mOwner->RemoveUIControl(mUIControl);
}

void ControlInterface::ControlElement::SetUpControl()
{
   std::string type = mInfo["type"].asString();
   mAttachedToUIControl = TheSynth->FindUIControl(mInfo["path"].asString());
   mTargetCable->SetTarget(mAttachedToUIControl);
   juce::String identifier;
   if (mAttachedToUIControl != nullptr)
   {
      identifier = mAttachedToUIControl->Path();
      identifier = identifier.replace("~", "-");
      std::vector<std::string> uiControlNames;
      for (auto* uiControl : mOwner->GetUIControls())
         uiControlNames.push_back(uiControl->Name());
      identifier = GetUniqueName(identifier.toStdString(), uiControlNames);
   }
   else
   {
      identifier = mInfo["identifier"].asString();
   }

   FloatSlider* attachedToFloatSlider = dynamic_cast<FloatSlider*>(mAttachedToUIControl);
   if (type == "floatslider" || attachedToFloatSlider != nullptr)
   {
      if (attachedToFloatSlider)
         mFloatVar = attachedToFloatSlider->GetVar();
      else
         mFloatVar = &mDummyFloat;
      if (mFloatSlider == nullptr)
         mFloatSlider = new FloatSlider(mOwner, identifier.toRawUTF8(), -1, -1, 100, 15, mFloatVar, -1, -1);
      else
         mFloatSlider->SetVar(mFloatVar);

      if (attachedToFloatSlider)
         mFloatSlider->CopyBehaviorFrom(attachedToFloatSlider);

      mUIControl = mFloatSlider;
   }

   IntSlider* attachedToIntSlider = dynamic_cast<IntSlider*>(mAttachedToUIControl);
   if (type == "intslider" || attachedToIntSlider != nullptr)
   {
      if (attachedToIntSlider)
         mIntVar = attachedToIntSlider->GetVar();
      else
         mIntVar = &mDummyInt;
      if (mIntSlider == nullptr)
         mIntSlider = new IntSlider(mOwner, identifier.toRawUTF8(), -1, -1, 100, 15, mIntVar, -1, -1);
      else
         mIntSlider->SetVar(mIntVar);
      mUIControl = mIntSlider;
   }

   Checkbox* attachedToCheckbox = dynamic_cast<Checkbox*>(mAttachedToUIControl);
   if (attachedToCheckbox)
   {
      mBoolVar = attachedToCheckbox->GetVar();
      if (mCheckbox == nullptr)
         mCheckbox = new Checkbox(mOwner, identifier.toRawUTF8(), -1, -1, mBoolVar);
      else
         mCheckbox->SetVar(mBoolVar);
      mUIControl = mCheckbox;
   }

   ClickButton* attachedToButton = dynamic_cast<ClickButton*>(mAttachedToUIControl);
   if (attachedToButton)
   {
      if (mButton == nullptr)
         mButton = new ClickButton(mOwner, identifier.toRawUTF8(), -1, -1);
      mUIControl = mButton;
   }

   DropdownList* attachedToDropdown = dynamic_cast<DropdownList*>(mAttachedToUIControl);
   if (attachedToDropdown)
   {
      mIntVar = attachedToDropdown->GetVar();
      if (mDropdown == nullptr)
         mDropdown = new DropdownList(mOwner, identifier.toRawUTF8(), -1, -1, mIntVar);
      else
         mDropdown->SetVar(mIntVar);
      mUIControl = mDropdown;
   }

   RadioButton* attachedToRadioButton = dynamic_cast<RadioButton*>(mAttachedToUIControl);
   if (attachedToRadioButton)
   {
      mIntVar = attachedToRadioButton->GetVar();
      if (mRadioButton == nullptr)
         mRadioButton = new RadioButton(mOwner, identifier.toRawUTF8(), -1, -1, mIntVar);
      else
         mRadioButton->SetVar(mIntVar);
      mUIControl = mRadioButton;
   }

   if (mUIControl != nullptr && mAttachedToUIControl != nullptr)
      mUIControl->SetControlVisualizer(mAttachedToUIControl->GetControlVisualizer());

   ApplyInfoToControl();
}

void ControlInterface::ControlElement::UpdateInfoFromControl()
{
   if (mAttachedToUIControl != nullptr)
      mInfo["path"] = mAttachedToUIControl->Path(false, false, mOwner->GetParent());
   else
      mInfo["path"] = "";

   if (mFloatSlider != nullptr)
   {
      mInfo["min"] = mFloatSlider->GetMin();
      mInfo["max"] = mFloatSlider->GetMax();
   }

   if (mIntSlider != nullptr)
   {
      mInfo["min"] = mIntSlider->GetMin();
      mInfo["max"] = mIntSlider->GetMax();
   }
}

float ControlInterface::ControlElement::GetInfo(std::string name, float defaultVal) const
{
   const auto& entry = mInfo[name];
   if (!entry.isNull())
      return entry.asFloat();
   return defaultVal;
}

bool ControlInterface::ControlElement::HasInfo(std::string name) const
{
   return !mInfo[name].isNull();
}

void ControlInterface::ControlElement::SetPosition(float x, float y)
{
   ofVec2f current = mUIControl->GetPosition(K(local));
   if (x != current.x || y != current.y || GetInfo("x", -1) != x || GetInfo("y", -1) != y)
   {
      mUIControl->SetPosition(x, y);
      mInfo["x"] = x;
      mInfo["y"] = y;
   }
}

void ControlInterface::ControlElement::ApplyInfoToControl()
{
   if (mUIControl)
   {
      mUIControl->SetOverrideDisplayName(mInfo["display_name"].asString());
      mUIControl->SetPosition(GetInfo("x", 3), GetInfo("y", 3));
   }

   if (mFloatSlider)
   {
      mFloatSlider->SetExtents(GetInfo("min", 0), GetInfo("max", 1));
      mFloatSlider->SetDimensions(GetInfo("width", 80), GetInfo("height", 15));
      mFloatSlider->SetDefaultValue(GetInfo("default_value", mFloatSlider->GetDefaultValue()));
   }

   if (mIntSlider)
   {
      mIntSlider->SetExtents((int)GetInfo("min", 0), (int)GetInfo("max", 1));
      mIntSlider->SetDimensions(GetInfo("width", 80), GetInfo("height", 15));
      mIntSlider->SetDefaultValue(GetInfo("default_value", mIntSlider->GetDefaultValue()));
   }

   if (mDropdown)
   {
      mDropdown->Clear();
      int numElements = mInfo["values"].size();
      for (int i = 0; i < numElements; ++i)
         mDropdown->AddLabel(mInfo["values"][i]["label"].asString(), mInfo["values"][i]["value"].asInt());
   }

   if (mRadioButton)
   {
      mRadioButton->Clear();
      int numElements = mInfo["values"].size();
      for (int i = 0; i < numElements; ++i)
         mRadioButton->AddLabel(mInfo["values"][i]["label"].asCString(), mInfo["values"][i]["value"].asInt());
   }

   if (mUIControl != nullptr && !IsSlider() && (HasInfo("width") || HasInfo("height")))
   {
      ofRectangle rect = mUIControl->GetRect(K(local));
      float width = GetInfo("width", rect.width);
      float height = GetInfo("height", rect.height);
      if (mCheckbox)
      {
         mCheckbox->SetDimensions(width, height);
         if (!HasInfo("width"))
            mCheckbox->SetOverrideDisplayName(mInfo["display_name"].asString()); //recalculate width to fit the new box size
      }
      if (mButton)
         mButton->SetDimensions(width, height);
      if (mDropdown)
         mDropdown->SetDimensions(width, height);
      if (mRadioButton)
         mRadioButton->SetDimensions(width, height);
   }
}

void ControlInterface::ControlElement::SaveState(FileStreamOut& out)
{
   UpdateInfoFromControl();

   out << mInfo.getRawString();
}

void ControlInterface::ControlElement::LoadState(FileStreamIn& in, int rev)
{
   std::string jsonString;
   in >> jsonString;
   mInfo.parse(jsonString);

   SetUpControl();
}