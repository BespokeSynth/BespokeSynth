/*
  ==============================================================================

    UserPrefsEditor.cpp
    Created: 12 Feb 2021 10:29:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "UserPrefsEditor.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

UserPrefsEditor::UserPrefsEditor()
{
}

UserPrefsEditor::~UserPrefsEditor()
{
}

void UserPrefsEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(165,50,200);
   DROPDOWN(mDeviceTypeDropdown, "devicetype", &mDeviceTypeIndex, 200);
   DROPDOWN(mAudioOutputDeviceDropdown, "audio_output_device", &mAudioOutputDeviceIndex, 350);
   DROPDOWN(mAudioInputDeviceDropdown, "audio_input_device", &mAudioInputDeviceIndex, 350);
   DROPDOWN(mSampleRateDropdown, "samplerate", &mSampleRateIndex, 100);
   DROPDOWN(mBufferSizeDropdown, "buffersize", &mBufferSizeIndex, 100);
   TEXTENTRY_NUM(mWindowWidthEntry, "width", 5, &mWindowWidth, 1, 10000);
   TEXTENTRY_NUM(mWindowHeightEntry, "height", 5, &mWindowHeight, 1, 10000);
   CHECKBOX(mSetWindowPositionCheckbox, "set position", &mSetWindowPosition);
   TEXTENTRY_NUM(mWindowPositionXEntry, "position_x", 5, &mWindowPositionX, -10000, 10000);
   TEXTENTRY_NUM(mWindowPositionYEntry, "position_y", 5, &mWindowPositionY, -10000, 10000);
   FLOATSLIDER(mZoomSlider, "zoom", &mZoom, .1f, 4);
   FLOATSLIDER(mScrollMultiplierVerticalSlider, "scroll_multiplier_vertical", &mScrollMultiplierVertical, -2, 2);
   FLOATSLIDER(mScrollMultiplierHorizontalSlider, "scroll_multiplier_horizontal", &mScrollMultiplierHorizontal, -2, 2);
   CHECKBOX(mAutosaveCheckbox, "autosave", &mAutosave);
   TEXTENTRY(mTooltipsFilePathEntry, "tooltips", 100, &mTooltipsFilePath);
   TEXTENTRY(mDefaultLayoutPathEntry, "layout", 100, &mDefaultLayoutPath);
   TEXTENTRY(mYoutubeDlPathEntry, "youtube-dl_path", 100, &mYoutubeDlPath);
   TEXTENTRY(mFfmpegPathEntry, "ffmpeg_path", 100, &mFfmpegPath);
   UIBLOCK_SHIFTDOWN();
   BUTTON(mSaveButton, "save and exit bespoke");
   BUTTON(mCancelButton, "cancel");
   ENDUIBLOCK(mWidth, mHeight);
   mWidth = 1150;


   mZoomSlider->SetShowName(false);
   mScrollMultiplierVerticalSlider->SetShowName(false);
   mScrollMultiplierHorizontalSlider->SetShowName(false);
}

void UserPrefsEditor::Show()
{
   SetShowing(true);
   mWindowWidth = TheSynth->GetUserPrefs()["width"].asInt();
   mWindowHeight = TheSynth->GetUserPrefs()["height"].asInt();
   mSetWindowPosition = !TheSynth->GetUserPrefs()["position_x"].isNull();
   if (mSetWindowPosition)
   {
      mWindowPositionX = TheSynth->GetUserPrefs()["position_x"].asInt();
      mWindowPositionY = TheSynth->GetUserPrefs()["position_y"].asInt();
   }
   else
   {
      mWindowPositionX = 100;
      mWindowPositionY = 100;
   }

   if (TheSynth->GetUserPrefs()["zoom"].isNull())
      mZoom = 1;
   else
      mZoom = TheSynth->GetUserPrefs()["zoom"].asDouble();

   if (TheSynth->GetUserPrefs()["scroll_multiplier_vertical"].isNull())
      mScrollMultiplierVertical = 1;
   else
      mScrollMultiplierVertical = TheSynth->GetUserPrefs()["scroll_multiplier_vertical"].asDouble();

   if (TheSynth->GetUserPrefs()["scroll_multiplier_horizontal"].isNull())
      mScrollMultiplierHorizontal = 1;
   else
      mScrollMultiplierHorizontal = TheSynth->GetUserPrefs()["scroll_multiplier_horizontal"].asDouble();

   if (TheSynth->GetUserPrefs()["tooltips"].isNull())
      mTooltipsFilePath = "internal/tooltips_eng.txt";
   else
      mTooltipsFilePath = TheSynth->GetUserPrefs()["tooltips"].asString();

   if (TheSynth->GetUserPrefs()["layout"].isNull())
      mDefaultLayoutPath = "layouts/blank.json";
   else
      mDefaultLayoutPath = TheSynth->GetUserPrefs()["layout"].asString();

   if (TheSynth->GetUserPrefs()["youtube-dl_path"].isNull())
      mYoutubeDlPath = "c:/youtube-dl/bin/youtube-dl.exe";
   else
      mYoutubeDlPath = TheSynth->GetUserPrefs()["youtube-dl_path"].asString();

   if (TheSynth->GetUserPrefs()["ffmpeg_path"].isNull())
      mFfmpegPath = "c:/ffmpeg/bin/ffmpeg.exe";
   else
      mFfmpegPath = TheSynth->GetUserPrefs()["ffmpeg_path"].asString();

   mWindowPositionXEntry->SetShowing(mSetWindowPosition);
   mWindowPositionYEntry->SetShowing(mSetWindowPosition);

   UpdateDropdowns({});
}

void UserPrefsEditor::UpdateDropdowns(vector<DropdownList*> toUpdate)
{
   AudioDeviceManager& deviceManager = TheSynth->GetGlobalManagers()->mDeviceManager;

   int i;

   if (toUpdate.empty() || VectorContains(mDeviceTypeDropdown, toUpdate))
   {
      mDeviceTypeIndex = -1;
      mDeviceTypeDropdown->Clear();
      mDeviceTypeDropdown->AddLabel("auto", -1);
      i = 0;
      for (auto* deviceType : deviceManager.getAvailableDeviceTypes())
      {
         mDeviceTypeDropdown->AddLabel(deviceType->getTypeName().toStdString(), i);
         if (deviceType == deviceManager.getCurrentDeviceTypeObject())
            mDeviceTypeIndex = i;
         ++i;
      }
   }

   auto* selectedDeviceType = mDeviceTypeIndex != -1 ? deviceManager.getAvailableDeviceTypes()[mDeviceTypeIndex] : deviceManager.getCurrentDeviceTypeObject();
   selectedDeviceType->scanForDevices();

   if (toUpdate.empty() || VectorContains(mAudioOutputDeviceDropdown, toUpdate))
   {
      mAudioOutputDeviceIndex = -1;
      mAudioOutputDeviceDropdown->Clear();
      mAudioOutputDeviceDropdown->AddLabel("none", -2);
      mAudioOutputDeviceDropdown->AddLabel("auto", -1);
      i = 0;
      for (auto outputDevice : selectedDeviceType->getDeviceNames())
      {
         mAudioOutputDeviceDropdown->AddLabel(outputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr && i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), false))
            mAudioOutputDeviceIndex = i;
         ++i;
      }

      if (mAudioOutputDeviceIndex == -1)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (mAudioOutputDeviceDropdown->GetLabel(j) == TheSynth->GetUserPrefs()["audio_output_device"].asString())
               mAudioOutputDeviceIndex = j;
         }
      }
   }

   if (toUpdate.empty() || VectorContains(mAudioInputDeviceDropdown, toUpdate))
   {
      mAudioInputDeviceIndex = -1;
      mAudioInputDeviceDropdown->Clear();
      mAudioInputDeviceDropdown->AddLabel("none", -2);
      mAudioInputDeviceDropdown->AddLabel("auto", -1);
      i = 0;
      for (auto inputDevice : selectedDeviceType->getDeviceNames(true))
      {
         mAudioInputDeviceDropdown->AddLabel(inputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr && i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), true))
            mAudioInputDeviceIndex = i;
         ++i;
      }

      if (mAudioInputDeviceIndex == -1)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (mAudioInputDeviceDropdown->GetLabel(j) == TheSynth->GetUserPrefs()["audio_input_device"].asString())
               mAudioInputDeviceIndex = j;
         }
      }
   }

   String outputDeviceName;
   if (mAudioOutputDeviceIndex >= 0)
      outputDeviceName = selectedDeviceType->getDeviceNames()[mAudioOutputDeviceIndex];
   else if (mAudioOutputDeviceIndex == -1)
      outputDeviceName = selectedDeviceType->getDeviceNames()[selectedDeviceType->getDefaultDeviceIndex(false)];

   String inputDeviceName;
   if (mAudioInputDeviceIndex >= 0)
      inputDeviceName = selectedDeviceType->getDeviceNames(true)[mAudioInputDeviceIndex];
   else if (mAudioInputDeviceIndex == -1)
      inputDeviceName = selectedDeviceType->getDeviceNames()[selectedDeviceType->getDefaultDeviceIndex(true)];

   auto* selectedDevice = selectedDeviceType->createDevice(outputDeviceName, inputDeviceName);
   
   if (toUpdate.empty() || VectorContains(mSampleRateDropdown, toUpdate))
   {
      mSampleRateIndex = -1;
      mSampleRateDropdown->Clear();
      i = 0;
      for (auto rate : selectedDevice->getAvailableSampleRates())
      {
         mSampleRateDropdown->AddLabel(ofToString(rate), i);
         if (rate == gSampleRate)
            mSampleRateIndex = i;
         ++i;
      }
   }

   if (toUpdate.empty() || VectorContains(mBufferSizeDropdown, toUpdate))
   {
      mBufferSizeIndex = -1;
      mBufferSizeDropdown->Clear();
      i = 0;
      for (auto bufferSize : selectedDevice->getAvailableBufferSizes())
      {
         mBufferSizeDropdown->AddLabel(ofToString(bufferSize), i);
         if (bufferSize == gBufferSize)
            mBufferSizeIndex = i;
         ++i;
      }
   }
}

void UserPrefsEditor::DrawModule()
{
   DrawTextNormal("editor for userprefs.json file", 3, 15);
   DrawTextNormal("any changes will not take effect until bespoke is restarted", 3, 35);

   for (auto* control : GetUIControls())
   {
      if (control->IsShowing() && control != mSaveButton && control != mCancelButton)
         DrawTextNormal(control->Name(), 3, control->GetPosition(K(local)).y + 12);
      control->Draw();
   }

   if (mDeviceTypeDropdown->GetLabel(mDeviceTypeIndex) == "DirectSound")
      DrawRightLabel(mDeviceTypeDropdown, "warning: DirectSound can cause crackle and strange behavior for some sample rates and buffer sizes", ofColor::yellow);

   if (mSampleRateDropdown->GetNumValues() == 0)
      DrawRightLabel(mSampleRateDropdown, "couldn't find a sample rate compatible between these output and input devices", ofColor::yellow);

   if (mBufferSizeDropdown->GetNumValues() == 0)
      DrawRightLabel(mBufferSizeDropdown, "couldn't find a buffer size compatible between these output and input devices", ofColor::yellow);
}

void UserPrefsEditor::DrawRightLabel(IUIControl* control, string text, ofColor color)
{
   ofRectangle rect = control->GetRect(true);
   ofPushStyle();
   ofSetColor(color);
   DrawTextNormal(text, rect.getMaxX()+5, rect.getMaxY()-3, 13);
   ofPopStyle();
}

void UserPrefsEditor::PrepareForSave()
{
   mSavePrefIndex = 0;
}

namespace
{
   string ToStringLeadingZeroes(int number) {
      char buffer[9];
      snprintf(buffer, sizeof(buffer), "%08d", number);
      return buffer;
   }
}

void UserPrefsEditor::UpdatePrefStr(ofxJSONElement& userPrefs, string prefName, string value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**"+ ToStringLeadingZeroes(mSavePrefIndex) + "**"+prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefInt(ofxJSONElement& userPrefs, string prefName, int value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefFloat(ofxJSONElement& userPrefs, string prefName, float value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefBool(ofxJSONElement& userPrefs, string prefName, bool value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::CleanUpSave(string& json)
{
   for (int i = 0; i < mSavePrefIndex; ++i)
   {
      ofStringReplace(json, "**" + ToStringLeadingZeroes(i) + "**", "", true);
   }
}

void UserPrefsEditor::ButtonClicked(ClickButton* button)
{
   if (button == mSaveButton)
   {
      auto userPrefs = TheSynth->GetUserPrefs();

      PrepareForSave();
      if (mDeviceTypeIndex != -1)
         UpdatePrefStr(userPrefs, "devicetype", mDeviceTypeDropdown->GetLabel(mDeviceTypeIndex));
      else
         userPrefs.removeMember("devicetype");
      UpdatePrefStr(userPrefs, "audio_output_device", mAudioOutputDeviceDropdown->GetLabel(mAudioOutputDeviceIndex));
      UpdatePrefStr(userPrefs, "audio_input_device", mAudioInputDeviceDropdown->GetLabel(mAudioInputDeviceIndex));
      UpdatePrefInt(userPrefs, "samplerate", ofToInt(mSampleRateDropdown->GetLabel(mSampleRateIndex)));
      UpdatePrefInt(userPrefs, "buffersize", ofToInt(mBufferSizeDropdown->GetLabel(mBufferSizeIndex)));
      UpdatePrefInt(userPrefs, "width", mWindowWidth);
      UpdatePrefInt(userPrefs, "height", mWindowHeight);
      if (mSetWindowPosition)
      {
         UpdatePrefInt(userPrefs, "position_x", mWindowPositionX);
         UpdatePrefInt(userPrefs, "position_y", mWindowPositionY);
      }
      else
      {
         userPrefs.removeMember("position_x");
         userPrefs.removeMember("position_y");
      }
      UpdatePrefFloat(userPrefs, "zoom", mZoom);
      UpdatePrefFloat(userPrefs, "scroll_multiplier_vertical", mScrollMultiplierVertical);
      UpdatePrefFloat(userPrefs, "scroll_multiplier_horizontal", mScrollMultiplierHorizontal);
      UpdatePrefBool(userPrefs, "autosave", mAutosave);
      UpdatePrefStr(userPrefs, "tooltips", mTooltipsFilePath);
      UpdatePrefStr(userPrefs, "layout", mDefaultLayoutPath);
      UpdatePrefStr(userPrefs, "youtube-dl_path", mYoutubeDlPath);
      UpdatePrefStr(userPrefs, "ffmpeg_path", mFfmpegPath);
      string output = userPrefs.getRawString(true);
      CleanUpSave(output);

      File file(TheSynth->GetUserPrefsPath(false));
      file.create();
      file.replaceWithText(output);

      JUCEApplicationBase::quit();
   }
   if (button == mCancelButton)
      SetShowing(false);
}

void UserPrefsEditor::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mSetWindowPositionCheckbox)
   {
      mWindowPositionXEntry->SetShowing(mSetWindowPosition);
      mWindowPositionYEntry->SetShowing(mSetWindowPosition);
   }
}

void UserPrefsEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void UserPrefsEditor::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void UserPrefsEditor::TextEntryComplete(TextEntry* entry)
{
}

void UserPrefsEditor::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mDeviceTypeDropdown)
   {
      UpdateDropdowns({ mAudioOutputDeviceDropdown, mAudioInputDeviceDropdown, mSampleRateDropdown, mBufferSizeDropdown });
   }

   if (list == mAudioOutputDeviceDropdown)
   {
      UpdateDropdowns({ mSampleRateDropdown, mBufferSizeDropdown });
   }

   if (list == mAudioInputDeviceDropdown)
   {
      UpdateDropdowns({ mSampleRateDropdown, mBufferSizeDropdown });
   }
}
