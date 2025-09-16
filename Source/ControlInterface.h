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

    ControlInterface.h
    Created: 6 May 2024
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"
#include "ofxJSONElement.h"
#include "CodeEntry.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "RadioButton.h"

class PatchCableSource;

class ControlInterface : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ICodeEntryListener, public IButtonListener, public IDropdownListener, public IRadioButtonListener
{
public:
   ControlInterface();
   virtual ~ControlInterface();
   static IDrawableModule* Create() { return new ControlInterface(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void KeyPressed(int key, bool isRepeat) override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   //ICodeEntryListener
   void ExecuteCode() override;
   std::pair<int, int> ExecuteBlock(int lineStart, int lineEnd) override { return std::pair<int, int>(); }
   void OnCodeUpdated() override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mWidth{ 130 };
   float mHeight{ 20 };

   class ControlElement
   {
   public:
      void Init(ControlInterface* owner);
      ~ControlElement();
      PatchCableSource* GetCableSource() const { return mTargetCable; }

      void SetUpControl();
      void UpdateInfoFromControl();
      void ApplyInfoToControl();
      float GetInfo(std::string name, float defaultVal) const;

      void SaveState(FileStreamOut& out);
      void LoadState(FileStreamIn& in, int rev);

      ofxJSONElement mInfo{};

      IUIControl* mAttachedToUIControl{ nullptr };
      IUIControl* mUIControl{ nullptr };
      FloatSlider* mFloatSlider{ nullptr };
      IntSlider* mIntSlider{ nullptr };
      Checkbox* mCheckbox{ nullptr };
      ClickButton* mButton{ nullptr };
      DropdownList* mDropdown{ nullptr };
      RadioButton* mRadioButton{ nullptr };
      float* mFloatVar{ nullptr };
      int* mIntVar{ nullptr };
      bool* mBoolVar{ nullptr };
      ControlInterface* mOwner{ nullptr };
      PatchCableSource* mTargetCable{ nullptr };
   };

   PatchCableSource* mAddControlCable{ nullptr };
   std::list<ControlElement*> mControls;
   ControlElement* mCurrentEditControl{ nullptr };
   CodeEntry* mControlEditorBox{ nullptr };
   bool mShowCables{ true };
   float mDummyFloat{ 0.0 };
   int mDummyInt{ 0 };
   bool mDummyBool{ false };
};
