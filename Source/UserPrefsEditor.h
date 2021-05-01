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
   string GetTitleLabel() override { return "settings"; }
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

   void UpdateDropdowns(vector<DropdownList*> toUpdate);
   void DrawRightLabel(IUIControl* control, string text, ofColor color);
   void PrepareForSave();
   void UpdatePrefStr(ofxJSONElement& userPrefs, string prefName, string value);
   void UpdatePrefInt(ofxJSONElement& userPrefs, string prefName, int value);
   void UpdatePrefFloat(ofxJSONElement& userPrefs, string prefName, float value);
   void UpdatePrefBool(ofxJSONElement& userPrefs, string prefName, bool value);
   void CleanUpSave(string& json);

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
   FloatSlider* mScrollMultiplierVerticalSlider;
   float mScrollMultiplierVertical;
   FloatSlider* mScrollMultiplierHorizontalSlider;
   float mScrollMultiplierHorizontal;
   Checkbox* mAutosaveCheckbox;
   bool mAutosave;
   TextEntry* mRecordBufferLengthEntry;
   float mRecordBufferLengthMinutes;
   TextEntry* mTooltipsFilePathEntry;
   string mTooltipsFilePath;
   TextEntry* mDefaultLayoutPathEntry;
   string mDefaultLayoutPath;
   TextEntry* mYoutubeDlPathEntry;
   string mYoutubeDlPath;
   TextEntry* mFfmpegPathEntry;
   string mFfmpegPath;
   ClickButton* mSaveButton;
   ClickButton* mCancelButton;

   float mWidth;
   float mHeight;
   int mSavePrefIndex;
};