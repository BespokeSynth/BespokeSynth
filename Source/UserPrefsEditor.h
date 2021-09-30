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

    UserPrefsEditor.h
    Created: 12 Feb 2021 10:29:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "TextEntry.h"
#include "DropdownList.h"
#include "ClickButton.h"

class UserPrefsEditor : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IDropdownListener, public IButtonListener
{
public:
   UserPrefsEditor();
   ~UserPrefsEditor();
   static IDrawableModule* Create() { return new UserPrefsEditor(); }

   void CreateUIControls() override;
   std::string GetTitleLabel() override { return "settings"; }
   bool AlwaysOnTop() override { return true; }
   bool CanMinimize() override { return false; }
   bool IsSingleton() const override { return true; }

   void Show();

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;

   bool IsSaveable() override { return false; }

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }

   void UpdateDropdowns(std::vector<DropdownList*> toUpdate);
   void DrawRightLabel(IUIControl* control, std::string text, ofColor color);
   void PrepareForSave();
   void UpdatePrefStr(ofxJSONElement& userPrefs, std::string prefName, std::string value);
   void UpdatePrefStrArray(ofxJSONElement& userPrefs, std::string prefName, std::vector<std::string> value);
   void UpdatePrefInt(ofxJSONElement& userPrefs, std::string prefName, int value);
   void UpdatePrefFloat(ofxJSONElement& userPrefs, std::string prefName, float value);
   void UpdatePrefBool(ofxJSONElement& userPrefs, std::string prefName, bool value);
   void CleanUpSave(std::string& json);

   DropdownList* mDeviceTypeDropdown;
   int mDeviceTypeIndex;
   DropdownList* mSampleRateDropdown;
   int mSampleRateIndex;
   DropdownList* mBufferSizeDropdown;
   int mBufferSizeIndex;
   DropdownList* mAudioOutputDeviceDropdown;
   int mAudioOutputDeviceIndex;
   DropdownList* mAudioInputDeviceDropdown;
   int mAudioInputDeviceIndex;
   TextEntry* mWindowWidthEntry;
   int mWindowWidth;
   TextEntry* mWindowHeightEntry;
   int mWindowHeight;
   Checkbox* mSetWindowPositionCheckbox;
   bool mSetWindowPosition;
   TextEntry* mWindowPositionXEntry;
   int mWindowPositionX;
   TextEntry* mWindowPositionYEntry;
   int mWindowPositionY;
   FloatSlider* mZoomSlider;
   float mZoom;
   FloatSlider* mUIScaleSlider;
   float mUIScale;
   FloatSlider* mScrollMultiplierVerticalSlider;
   float mScrollMultiplierVertical;
   FloatSlider* mScrollMultiplierHorizontalSlider;
   float mScrollMultiplierHorizontal;
   Checkbox* mAutosaveCheckbox;
   bool mAutosave;
   TextEntry* mRecordingsPathEntry;
   std::string mRecordingsPath;
   TextEntry* mRecordBufferLengthEntry;
   float mRecordBufferLengthMinutes;
   TextEntry* mTooltipsFilePathEntry;
   std::string mTooltipsFilePath;
   TextEntry* mDefaultLayoutPathEntry;
   std::string mDefaultLayoutPath;
   TextEntry* mYoutubeDlPathEntry;
   std::string mYoutubeDlPath;
   TextEntry* mFfmpegPathEntry;
   std::string mFfmpegPath;
   TextEntry* mVstSearchDirsEntry;
   std::string mVstSearchDirs;
   Checkbox* mShowTooltipsOnLoadCheckbox;
   bool mShowTooltipsOnLoad;
   ClickButton* mSaveButton;
   ClickButton* mCancelButton;

   float mWidth;
   float mHeight;
   int mSavePrefIndex;
};
