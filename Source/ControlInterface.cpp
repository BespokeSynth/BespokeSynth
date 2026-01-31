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
}

ControlInterface::~ControlInterface()
{
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
      IUIControl* targetUIControl = dynamic_cast<IUIControl*>(cableSource->GetTarget());

      for (auto* control : mControls)
      {
         if (control->GetCableSource() == cableSource)
         {
            if (targetUIControl != nullptr)
               repatched = true;

            RemovePatchCableSource(cableSource);
            if (control->mUIControl != nullptr)
               RemoveUIControl(control->mUIControl);
            mControls.remove(control);
            break;
         }
      }

      if (cableSource == mAddControlCable || repatched)
      {
         if (targetUIControl != nullptr)
         {
            mAddControlCable->Clear();

            ofVec2f controlPos = FindNewUIControlPos();

            ControlElement* control = new ControlElement();
            mControls.push_back(control);
            control->Init(this);

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
            }

            IntSlider* intSlider = dynamic_cast<IntSlider*>(targetUIControl);
            if (intSlider)
            {
               control->mInfo["type"] = "intslider";
               control->mInfo["min"] = intSlider->GetMin();
               control->mInfo["max"] = intSlider->GetMax();
               control->mInfo["width"] = kDefaultSliderWidth;
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

ControlInterface::ControlElement* ControlInterface::FindOrCreateNewControl(std::string identifier, std::string type, bool& isNewControl)
{
   ControlElement* control = nullptr;
   for (auto* existingControl : mControls)
   {
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
            //we already have a control with this name but it's a different type, delete it and make a new one
            RemovePatchCableSource(control->mTargetCable);
            if (control->mUIControl != nullptr)
               RemoveUIControl(control->mUIControl);
            mControls.remove(control);
         }
         break;
      }
   }

   if (control == nullptr)
   {
      control = new ControlElement();
      mControls.push_back(control);
      control->Init(this);
      control->mInfo["type"] = type;
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

   control->mInfo["path"] = "";
   control->mInfo["display_name"] = name;
   control->mInfo["identifier"] = name;
   control->mInfo["x"] = controlPos.x;
   control->mInfo["y"] = controlPos.y;
   control->mInfo["min"] = min;
   control->mInfo["max"] = max;
   control->mInfo["width"] = kDefaultSliderWidth;

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

   control->mInfo["path"] = "";
   control->mInfo["display_name"] = name;
   control->mInfo["identifier"] = name;
   control->mInfo["x"] = controlPos.x;
   control->mInfo["y"] = controlPos.y;
   control->mInfo["min"] = min;
   control->mInfo["max"] = max;
   control->mInfo["width"] = kDefaultSliderWidth;

   control->SetUpControl();

   return control->mIntSlider;
}

void ControlInterface::ClearAllControls()
{
   for (auto* control : mControls)
   {
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
         mFloatSlider->SetMode(attachedToFloatSlider->GetMode());

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
      mFloatSlider->SetDimensions(GetInfo("width", 80), 15);
   }

   if (mIntSlider)
   {
      mIntSlider->SetExtents((int)GetInfo("min", 0), (int)GetInfo("max", 1));
      mIntSlider->SetDimensions(GetInfo("width", 80), 15);
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