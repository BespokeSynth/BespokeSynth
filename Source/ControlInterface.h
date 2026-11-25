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
#include "TextEntry.h"

class PatchCableSource;

class ControlInterface : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ICodeEntryListener, public IButtonListener, public IDropdownListener, public IRadioButtonListener, public ITextEntryListener
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
   void MouseReleased() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   FloatSlider* AddFloatSlider(std::string name, float defaultVal, float min, float max);
   IntSlider* AddIntSlider(std::string name, int defaultVal, int min, int max);
   void ClearAllControls();

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   //present our controls to control surfaces in our own specified order
   bool HasPush2OverrideControls() const override { return true; }
   void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const override;

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
   void Poll() override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   static const int kDefaultSliderWidth{ 120 };

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
      bool HasInfo(std::string name) const;
      bool IsSlider() const { return mFloatSlider != nullptr || mIntSlider != nullptr; }
      void SetPosition(float x, float y);

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

      float mDummyFloat{ 0.0 };
      int mDummyInt{ 0 };
      bool mDummyBool{ false };
   };

   ofVec2f FindNewUIControlPos();
   ControlElement* CreateControl(std::list<ControlElement*>::iterator insertPos);
   ControlElement* FindOrCreateNewControl(std::string identifier, std::string type, bool& isNewControl);

   //edit mode
   void SetEditMode(bool editMode);
   void SetSelectedControl(ControlElement* control);
   void MoveSelectedControl(int direction);
   void AutoPositionControls();
   void UpdateEditorFields();
   void DrawEditor();
   void OnControlRemoved(ControlElement* control);
   ControlElement* GetControlAt(float x, float y) const;
   template <class T>
   T* CreateEditorControl(T* control);

   PatchCableSource* mAddControlCable{ nullptr };
   std::list<ControlElement*> mControls;
   ControlElement* mCurrentEditControl{ nullptr };
   CodeEntry* mControlEditorBox{ nullptr };
   bool mShowCables{ true };

   bool mEditMode{ false };
   bool mAutoPosition{ true };
   ControlElement* mSelectedControl{ nullptr };
   ControlElement* mDragControl{ nullptr };
   ofVec2f mDragStartMousePos;
   ofVec2f mDragStartControlPos;
   bool mHasDragged{ false };
   float mEditorY{ 0 };
   std::vector<IUIControl*> mEditorControls; //controls used in edit mode. these aren't in the module's list of controls.
   Checkbox* mAutoPositionCheckbox{ nullptr };
   TextEntry* mEditNameEntry{ nullptr };
   TextEntry* mEditMinEntry{ nullptr };
   TextEntry* mEditMaxEntry{ nullptr };
   TextEntry* mEditWidthEntry{ nullptr };
   TextEntry* mEditHeightEntry{ nullptr };
   ClickButton* mMoveUpButton{ nullptr };
   ClickButton* mMoveDownButton{ nullptr };
   std::string mEditName;
   float mEditMin{ 0 };
   float mEditMax{ 1 };
   int mEditWidth{ 0 };
   int mEditHeight{ 0 };
};
