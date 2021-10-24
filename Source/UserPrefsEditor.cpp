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

    UserPrefsEditor.cpp
    Created: 12 Feb 2021 10:29:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "UserPrefsEditor.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_gui_basics/juce_gui_basics.h"

UserPrefsEditor::UserPrefsEditor()
{
}

UserPrefsEditor::~UserPrefsEditor()
{
}

void UserPrefsEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(175,50,200);
   mDeviceType.SetUpControl(this);
   mAudioOutputDevice.SetUpControl(this);
   mAudioInputDevice.SetUpControl(this);
   mSampleRate.SetUpControl(this);
   mBufferSize.SetUpControl(this);
   TEXTENTRY_NUM(mWindowWidthEntry, "width", 5, &mWindowWidth, 1, 10000);
   TEXTENTRY_NUM(mWindowHeightEntry, "height", 5, &mWindowHeight, 1, 10000);
   CHECKBOX(mSetWindowPositionCheckbox, "set position", &mSetWindowPosition);
   TEXTENTRY_NUM(mWindowPositionXEntry, "position_x", 5, &mWindowPositionX, -10000, 10000);
   TEXTENTRY_NUM(mWindowPositionYEntry, "position_y", 5, &mWindowPositionY, -10000, 10000);
   FLOATSLIDER(mZoomSlider, "zoom", &mZoom, .25f, 2);
   FLOATSLIDER(mUIScaleSlider, "ui_scale", &mUIScale, .25f, 2);
   FLOATSLIDER(mScrollMultiplierVerticalSlider, "scroll_multiplier_vertical", &mScrollMultiplierVertical, -2, 2);
   FLOATSLIDER(mScrollMultiplierHorizontalSlider, "scroll_multiplier_horizontal", &mScrollMultiplierHorizontal, -2, 2);
   CHECKBOX(mAutosaveCheckbox, "autosave", &mAutosave);
   TEXTENTRY(mRecordingsPathEntry, "recordings_path", 70, &mRecordingsPath);
   TEXTENTRY_NUM(mRecordBufferLengthEntry, "record_buffer_length_minutes", 5, &mRecordBufferLengthMinutes, 1, 120);
   TEXTENTRY(mTooltipsFilePathEntry, "tooltips", 100, &mTooltipsFilePath);
   TEXTENTRY(mDefaultLayoutPathEntry, "layout", 100, &mDefaultLayoutPath);
   TEXTENTRY(mYoutubeDlPathEntry, "youtube-dl_path", 100, &mYoutubeDlPath);
   TEXTENTRY(mFfmpegPathEntry, "ffmpeg_path", 100, &mFfmpegPath);
   CHECKBOX(mShowTooltipsOnLoadCheckbox, "show_tooltips_on_load", &mShowTooltipsOnLoad);
   CHECKBOX(mShowMinimapCheckbox, "show_minimap", &mShowMinimap);
   UIBLOCK_SHIFTDOWN();
   BUTTON(mSaveButton, "save and exit bespoke");
   BUTTON(mCancelButton, "cancel");
   ENDUIBLOCK0();

   mZoomSlider->SetShowName(false);
   mUIScaleSlider->SetShowName(false);
   mScrollMultiplierVerticalSlider->SetShowName(false);
   mScrollMultiplierHorizontalSlider->SetShowName(false);
}

void UserPrefsEditor::Show()
{
   SetShowing(true);
   if (TheSynth->GetUserPrefs()["width"].isNull())
      mWindowWidth = 1700;
   else
      mWindowWidth = TheSynth->GetUserPrefs()["width"].asInt();
   if (TheSynth->GetUserPrefs()["height"].isNull())
      mWindowHeight = 1100;
   else
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
   
   if (TheSynth->GetUserPrefs()["ui_scale"].isNull())
      mUIScale = 1;
   else
      mUIScale = TheSynth->GetUserPrefs()["ui_scale"].asDouble();

   if (TheSynth->GetUserPrefs()["scroll_multiplier_vertical"].isNull())
      mScrollMultiplierVertical = 1;
   else
      mScrollMultiplierVertical = TheSynth->GetUserPrefs()["scroll_multiplier_vertical"].asDouble();

   if (TheSynth->GetUserPrefs()["scroll_multiplier_horizontal"].isNull())
      mScrollMultiplierHorizontal = 1;
   else
      mScrollMultiplierHorizontal = TheSynth->GetUserPrefs()["scroll_multiplier_horizontal"].asDouble();

   if (TheSynth->GetUserPrefs()["recordings_path"].isNull())
      mRecordingsPath = "recordings/";
   else
      mRecordingsPath = TheSynth->GetUserPrefs()["recordings_path"].asString();

   if (TheSynth->GetUserPrefs()["autosave"].isNull())
      mAutosave = false;
   else
      mAutosave = TheSynth->GetUserPrefs()["autosave"].asBool();

   if (TheSynth->GetUserPrefs()["record_buffer_length_minutes"].isNull())
      mRecordBufferLengthMinutes = 30;
   else
      mRecordBufferLengthMinutes = TheSynth->GetUserPrefs()["record_buffer_length_minutes"].asDouble();

   if (TheSynth->GetUserPrefs()["tooltips"].isNull())
      mTooltipsFilePath = "tooltips_eng.txt";
   else
      mTooltipsFilePath = TheSynth->GetUserPrefs()["tooltips"].asString();

   if (TheSynth->GetUserPrefs()["layout"].isNull())
      mDefaultLayoutPath = "layouts/blank.json";
   else
      mDefaultLayoutPath = TheSynth->GetUserPrefs()["layout"].asString();

   if (TheSynth->GetUserPrefs()["youtube-dl_path"].isNull())
   {
#if BESPOKE_MAC
      mYoutubeDlPath = "/opt/local/bin/youtube-dl";
#elif BESPOKE_LINUX
      mYoutubeDlPath = "/usr/bin/youtube-dl";
#else
      mYoutubeDlPath = "c:/youtube-dl/bin/youtube-dl.exe";
#endif
   }
   else
      mYoutubeDlPath = TheSynth->GetUserPrefs()["youtube-dl_path"].asString();

   if (TheSynth->GetUserPrefs()["ffmpeg_path"].isNull())
   {
#if BESPOKE_MAC
      mFfmpegPath = "/opt/local/bin/ffmpeg";
#elif BESPOKE_LINUX
      mFfmpegPath = "/usr/bin/ffmpeg";
#else
      mFfmpegPath = "c:/ffmpeg/bin/ffmpeg.exe";
#endif
   }
   else
      mFfmpegPath = TheSynth->GetUserPrefs()["ffmpeg_path"].asString();

   if (TheSynth->GetUserPrefs()["show_tooltips_on_load"].isNull())
      mShowTooltipsOnLoad = true;
   else
      mShowTooltipsOnLoad = TheSynth->GetUserPrefs()["show_tooltips_on_load"].asBool();

   if (TheSynth->GetUserPrefs()["show_minimap"].isNull())
      mShowMinimap = false;
   else
      mShowMinimap = TheSynth->GetUserPrefs()["show_minimap"].asBool();

   mWindowPositionXEntry->SetShowing(mSetWindowPosition);
   mWindowPositionYEntry->SetShowing(mSetWindowPosition);

   UpdateDropdowns({});
}

void UserPrefsEditor::UpdateDropdowns(std::vector<DropdownList*> toUpdate)
{
   auto& deviceManager = TheSynth->GetAudioDeviceManager();

   int i;

   if (toUpdate.empty() || VectorContains(mDeviceType.GetDropdown(), toUpdate))
   {
      mDeviceType.GetValue() = -1;
      mDeviceType.GetDropdown()->Clear();
      mDeviceType.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto* deviceType : deviceManager.getAvailableDeviceTypes())
      {
         mDeviceType.GetDropdown()->AddLabel(deviceType->getTypeName().toStdString(), i);
         if (deviceType == deviceManager.getCurrentDeviceTypeObject())
            mDeviceType.GetValue() = i;
         ++i;
      }
   }

   auto* selectedDeviceType = mDeviceType.GetValue() != -1 ? deviceManager.getAvailableDeviceTypes()[mDeviceType.GetValue()] : deviceManager.getCurrentDeviceTypeObject();
   selectedDeviceType->scanForDevices();

   if (toUpdate.empty() || VectorContains(mAudioOutputDevice.GetDropdown(), toUpdate))
   {
      mAudioOutputDevice.GetValue() = -1;
      mAudioOutputDevice.GetDropdown()->Clear();
      mAudioOutputDevice.GetDropdown()->AddLabel("none", -2);
      mAudioOutputDevice.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto outputDevice : selectedDeviceType->getDeviceNames())
      {
         mAudioOutputDevice.GetDropdown()->AddLabel(outputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr && i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), false))
            mAudioOutputDevice.GetValue() = i;
         ++i;
      }

      if (mAudioOutputDevice.GetValue() == -1)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (mAudioOutputDevice.GetDropdown()->GetLabel(j) == TheSynth->GetUserPrefs()["audio_output_device"].asString())
               mAudioOutputDevice.GetValue() = j;
         }
      }
   }

   if (toUpdate.empty() || VectorContains(mAudioInputDevice.GetDropdown(), toUpdate))
   {
      mAudioInputDevice.GetValue() = -1;
      if (TheSynth->GetUserPrefs()["audio_input_device"].isNull())
         mAudioInputDevice.GetValue() = -2;  //default to "none"
      mAudioInputDevice.GetDropdown()->Clear();
      mAudioInputDevice.GetDropdown()->AddLabel("none", -2);
      mAudioInputDevice.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto inputDevice : selectedDeviceType->getDeviceNames(true))
      {
         mAudioInputDevice.GetDropdown()->AddLabel(inputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr && i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), true))
            mAudioInputDevice.GetValue() = i;
         ++i;
      }

      if (mAudioInputDevice.GetValue() < 0)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (mAudioInputDevice.GetDropdown()->GetLabel(j) == TheSynth->GetUserPrefs()["audio_input_device"].asString())
               mAudioInputDevice.GetValue() = j;
         }
      }
   }

   juce::String outputDeviceName;
   if (mAudioOutputDevice.GetValue() >= 0)
      outputDeviceName = selectedDeviceType->getDeviceNames()[mAudioOutputDevice.GetValue()];
   else if (mAudioOutputDevice.GetValue() == -1)
      outputDeviceName = selectedDeviceType->getDeviceNames()[selectedDeviceType->getDefaultDeviceIndex(false)];

   juce::String inputDeviceName;
   if (selectedDeviceType->hasSeparateInputsAndOutputs())
   {
      if (mAudioInputDevice.GetValue() >= 0)
         inputDeviceName = selectedDeviceType->getDeviceNames(true)[mAudioInputDevice.GetValue()];
      else if (mAudioInputDevice.GetValue() == -1)
         inputDeviceName = selectedDeviceType->getDeviceNames()[selectedDeviceType->getDefaultDeviceIndex(true)];
   }
   else
   {
      inputDeviceName = outputDeviceName;
   }

   auto* selectedDevice = deviceManager.getCurrentAudioDevice();
   auto setup = deviceManager.getAudioDeviceSetup();
   if (selectedDevice == nullptr || selectedDevice->getTypeName() != selectedDeviceType->getTypeName() || setup.outputDeviceName != outputDeviceName || setup.inputDeviceName != inputDeviceName)
      selectedDevice = selectedDeviceType->createDevice(outputDeviceName, inputDeviceName);

   if (selectedDevice == nullptr)
      return;
   
   if (toUpdate.empty() || VectorContains(mSampleRate.GetDropdown(), toUpdate))
   {
      mSampleRate.GetValue() = -1;
      mSampleRate.GetDropdown()->Clear();
      i = 0;
      for (auto rate : selectedDevice->getAvailableSampleRates())
      {
         mSampleRate.GetDropdown()->AddLabel(ofToString(rate), i);
         if (rate == gSampleRate)
            mSampleRate.GetValue() = i;
         ++i;
      }
   }

   if (toUpdate.empty() || VectorContains(mBufferSize.GetDropdown(), toUpdate))
   {
      mBufferSize.GetValue() = -1;
      mBufferSize.GetDropdown()->Clear();
      i = 0;
      for (auto bufferSize : selectedDevice->getAvailableBufferSizes())
      {
         mBufferSize.GetDropdown()->AddLabel(ofToString(bufferSize), i);
         if (bufferSize == gBufferSize)
            mBufferSize.GetValue() = i;
         ++i;
      }
   }

   if (selectedDevice != deviceManager.getCurrentAudioDevice())
      delete selectedDevice;
}

void UserPrefsEditor::DrawModule()
{
   DrawTextNormal("editor for userprefs.json file", 3, 15);
   DrawTextNormal("any changes will not take effect until bespoke is restarted", 3, 35);

   auto& deviceManager = TheSynth->GetAudioDeviceManager();
   auto* selectedDeviceType = mDeviceType.GetValue() != -1 ? deviceManager.getAvailableDeviceTypes()[mDeviceType.GetValue()] : deviceManager.getCurrentDeviceTypeObject();
   mAudioInputDevice.GetDropdown()->SetShowing(selectedDeviceType->hasSeparateInputsAndOutputs());

   int controlX = 175;
   int controlY = 50;
   for (auto* control : GetUIControls())
   {
      control->SetPosition(controlX, controlY);
      if (control->IsShowing() && control != mSaveButton && control != mCancelButton)
         DrawTextNormal(control->Name(), 3, control->GetPosition(K(local)).y + 12);
      control->Draw();

      controlY += 17;
   }
   mWidth = 1150;
   mHeight = controlY + 20;

   if (mDeviceType.GetDropdown()->GetLabel(mDeviceType.GetValue()) == "DirectSound")
      DrawRightLabel(mDeviceType.GetDropdown(), "warning: DirectSound can cause crackle and strange behavior for some sample rates and buffer sizes", ofColor::yellow);

   if (mSampleRate.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType->hasSeparateInputsAndOutputs())
         DrawRightLabel(mSampleRate.GetDropdown(), "couldn't find a sample rate compatible between these output and input devices", ofColor::yellow);
      else
         DrawRightLabel(mSampleRate.GetDropdown(), "couldn't find any sample rates for this device, for some reason (is it plugged in?)", ofColor::yellow);
   }

   if (mBufferSize.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType->hasSeparateInputsAndOutputs())
         DrawRightLabel(mBufferSize.GetDropdown(), "couldn't find a buffer size compatible between these output and input devices", ofColor::yellow);
      else
         DrawRightLabel(mBufferSize.GetDropdown(), "couldn't find any buffer sizes for this device, for some reason (is it plugged in?)", ofColor::yellow);
   }

   DrawRightLabel(mWindowWidthEntry, "(currently: " + ofToString(ofGetWidth()) + ")", ofColor::white);
   DrawRightLabel(mWindowHeightEntry, "(currently: " + ofToString(ofGetHeight()) + ")", ofColor::white);
   if (mSetWindowPosition)
   {
      auto pos = TheSynth->GetMainComponent()->getTopLevelComponent()->getScreenPosition();

      DrawRightLabel(mWindowPositionXEntry, "(currently: " + ofToString(pos.x) + ")", ofColor::white);
      DrawRightLabel(mWindowPositionYEntry, "(currently: " + ofToString(pos.y) + ")", ofColor::white);
   }
   DrawRightLabel(mZoomSlider, "(currently: " + ofToString(gDrawScale) + ")", ofColor::white);
   DrawRightLabel(mRecordingsPathEntry, "(default: recordings/)", ofColor::white);
}

void UserPrefsEditor::DrawRightLabel(IUIControl* control, std::string text, ofColor color)
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
   std::string ToStringLeadingZeroes(int number) {
      char buffer[9];
      snprintf(buffer, sizeof(buffer), "%08d", number);
      return buffer;
   }
}

void UserPrefsEditor::UpdatePrefStr(ofxJSONElement& userPrefs, std::string prefName, std::string value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**"+ ToStringLeadingZeroes(mSavePrefIndex) + "**"+prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefStrArray(ofxJSONElement& userPrefs, std::string prefName, std::vector<std::string> value)
{
   userPrefs.removeMember(prefName);
   ofxJSONElement strArray;
   for (std::string str : value)
      strArray.append(str);
   userPrefs["**"+ ToStringLeadingZeroes(mSavePrefIndex) + "**"+prefName] = strArray;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefInt(ofxJSONElement& userPrefs, std::string prefName, int value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefFloat(ofxJSONElement& userPrefs, std::string prefName, float value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::UpdatePrefBool(ofxJSONElement& userPrefs, std::string prefName, bool value)
{
   userPrefs.removeMember(prefName);
   userPrefs["**" + ToStringLeadingZeroes(mSavePrefIndex) + "**" + prefName] = value;
   ++mSavePrefIndex;
}

void UserPrefsEditor::CleanUpSave(std::string& json)
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
      if (mDeviceType.GetValue() != -1)
         UpdatePrefStr(userPrefs, "devicetype", mDeviceType.GetDropdown()->GetLabel(mDeviceType.GetValue()));
      else
         userPrefs.removeMember("devicetype");
      UpdatePrefStr(userPrefs, "audio_output_device", mAudioOutputDevice.GetDropdown()->GetLabel(mAudioOutputDevice.GetValue()));
      UpdatePrefStr(userPrefs, "audio_input_device", mAudioInputDevice.GetDropdown()->GetLabel(mAudioInputDevice.GetValue()));
      UpdatePrefInt(userPrefs, "samplerate", ofToInt(mSampleRate.GetDropdown()->GetLabel(mSampleRate.GetValue())));
      UpdatePrefInt(userPrefs, "buffersize", ofToInt(mBufferSize.GetDropdown()->GetLabel(mBufferSize.GetValue())));
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
      UpdatePrefFloat(userPrefs, "ui_scale", mUIScale);
      UpdatePrefFloat(userPrefs, "scroll_multiplier_vertical", mScrollMultiplierVertical);
      UpdatePrefFloat(userPrefs, "scroll_multiplier_horizontal", mScrollMultiplierHorizontal);
      UpdatePrefBool(userPrefs, "autosave", mAutosave);
      UpdatePrefStr(userPrefs, "recordings_path", mRecordingsPath);
      UpdatePrefFloat(userPrefs, "record_buffer_length_minutes", mRecordBufferLengthMinutes);
      UpdatePrefStr(userPrefs, "tooltips", mTooltipsFilePath);
      UpdatePrefStr(userPrefs, "layout", mDefaultLayoutPath);
      UpdatePrefStr(userPrefs, "youtube-dl_path", mYoutubeDlPath);
      UpdatePrefStr(userPrefs, "ffmpeg_path", mFfmpegPath);
      UpdatePrefBool(userPrefs, "show_tooltips_on_load", mShowTooltipsOnLoad);
      UpdatePrefBool(userPrefs, "show_minimap", mShowMinimap);

      std::string output = userPrefs.getRawString(true);
      CleanUpSave(output);

      juce::File file(TheSynth->GetUserPrefsPath(false));
      file.create();
      file.replaceWithText(output);

      juce::JUCEApplicationBase::quit();
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
   if (mUIScaleSlider && !TheSynth->IsLoadingState())
      TheSynth->SetUIScale(mUIScale);
}

void UserPrefsEditor::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void UserPrefsEditor::TextEntryComplete(TextEntry* entry)
{
}

void UserPrefsEditor::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mDeviceType.GetDropdown())
   {
      UpdateDropdowns({ mAudioOutputDevice.GetDropdown(), mAudioInputDevice.GetDropdown(), mSampleRate.GetDropdown(), mBufferSize.GetDropdown() });
   }

   if (list == mAudioOutputDevice.GetDropdown())
   {
      UpdateDropdowns({ mSampleRate.GetDropdown(), mBufferSize.GetDropdown() });
   }

   if (list == mAudioInputDevice.GetDropdown())
   {
      UpdateDropdowns({ mSampleRate.GetDropdown(), mBufferSize.GetDropdown() });
   }
}
