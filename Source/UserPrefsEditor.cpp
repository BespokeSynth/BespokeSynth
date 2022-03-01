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
#include "UserPrefs.h"

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
   SetName("settings");

   IDrawableModule::CreateUIControls();

   for (auto* pref : UserPrefs.mUserPrefs)
      pref->SetUpControl(this);

   mCategorySelector = new RadioButton(this, "category", 5, 10, (int*)(&mCategory), kRadioHorizontal);
   mSaveButton = new ClickButton(this, "save", -1, -1);
   mCancelButton = new ClickButton(this, "cancel", -1, -1);

   mCategorySelector->AddLabel("general", (int)UserPrefCategory::General);
   mCategorySelector->AddLabel("graphics", (int)UserPrefCategory::Graphics);
   mCategorySelector->AddLabel("paths", (int)UserPrefCategory::Paths);

   std::array<int, 5> oversampleAmounts = { 1, 2, 4, 8, 16 };
   for (int oversample : oversampleAmounts)
   {
      UserPrefs.oversampling.GetDropdown()->AddLabel(ofToString(oversample), oversample);
      if (UserPrefs.oversampling.Get() == oversample)
         UserPrefs.oversampling.GetIndex() = oversample;
   }
}

void UserPrefsEditor::Show()
{
   UpdateDropdowns({});
   SetShowing(true);

   if (TheSynth->HasFatalError())
   {
      mSaveButton->SetLabel("save and exit");
      mCancelButton->SetShowing(false);
   }
}

void UserPrefsEditor::CreatePrefsFileIfNonexistent()
{
   UpdateDropdowns({});

   if (!juce::File(TheSynth->GetUserPrefsPath()).existsAsFile())
   {
      Save();
      UserPrefs.mUserPrefsFile.open(TheSynth->GetUserPrefsPath());
   }
}

void UserPrefsEditor::UpdateDropdowns(std::vector<DropdownList*> toUpdate)
{
   auto& deviceManager = TheSynth->GetAudioDeviceManager();

   int i;

   if (toUpdate.empty() || VectorContains(UserPrefs.devicetype.GetDropdown(), toUpdate))
   {
      UserPrefs.devicetype.GetIndex() = -1;
      UserPrefs.devicetype.GetDropdown()->Clear();
      UserPrefs.devicetype.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto* deviceType : deviceManager.getAvailableDeviceTypes())
      {
         UserPrefs.devicetype.GetDropdown()->AddLabel(deviceType->getTypeName().toStdString(), i);
         if (deviceType == deviceManager.getCurrentDeviceTypeObject() &&
             UserPrefs.devicetype.Get() != "auto")
            UserPrefs.devicetype.GetIndex() = i;
         ++i;
      }
   }

   auto* selectedDeviceType = UserPrefs.devicetype.GetIndex() != -1 ? deviceManager.getAvailableDeviceTypes()[UserPrefs.devicetype.GetIndex()] : deviceManager.getCurrentDeviceTypeObject();
   selectedDeviceType->scanForDevices();

   if (toUpdate.empty() || VectorContains(UserPrefs.audio_output_device.GetDropdown(), toUpdate))
   {
      UserPrefs.audio_output_device.GetIndex() = -1;
      UserPrefs.audio_output_device.GetDropdown()->Clear();
      UserPrefs.audio_output_device.GetDropdown()->AddLabel("none", -2);
      UserPrefs.audio_output_device.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto outputDevice : selectedDeviceType->getDeviceNames())
      {
         UserPrefs.audio_output_device.GetDropdown()->AddLabel(outputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr &&
             i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), false) &&
             UserPrefs.audio_output_device.Get() != "auto")
            UserPrefs.audio_output_device.GetIndex() = i;
         ++i;
      }

      if (UserPrefs.audio_output_device.GetIndex() == -1)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (UserPrefs.audio_output_device.GetDropdown()->GetLabel(j) == UserPrefs.audio_output_device.Get())
               UserPrefs.audio_output_device.GetIndex() = j;
         }
      }
   }

   if (toUpdate.empty() || VectorContains(UserPrefs.audio_input_device.GetDropdown(), toUpdate))
   {
      UserPrefs.audio_input_device.GetIndex() = -1;
      if (UserPrefs.audio_input_device.Get() == "none")
         UserPrefs.audio_input_device.GetIndex() = -2;
      UserPrefs.audio_input_device.GetDropdown()->Clear();
      UserPrefs.audio_input_device.GetDropdown()->AddLabel("none", -2);
      UserPrefs.audio_input_device.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto inputDevice : selectedDeviceType->getDeviceNames(true))
      {
         UserPrefs.audio_input_device.GetDropdown()->AddLabel(inputDevice.toStdString(), i);
         if (deviceManager.getCurrentAudioDevice() != nullptr &&
             i == selectedDeviceType->getIndexOfDevice(deviceManager.getCurrentAudioDevice(), true) &&
             UserPrefs.audio_input_device.Get() != "auto")
            UserPrefs.audio_input_device.GetIndex() = i;
         ++i;
      }

      if (UserPrefs.audio_input_device.GetIndex() < 0)   //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (UserPrefs.audio_input_device.GetDropdown()->GetLabel(j) == UserPrefs.audio_input_device.Get())
               UserPrefs.audio_input_device.GetIndex() = j;
         }
      }
   }

   juce::String outputDeviceName;
   if (UserPrefs.audio_output_device.GetIndex() >= 0)
      outputDeviceName = selectedDeviceType->getDeviceNames()[UserPrefs.audio_output_device.GetIndex()];
   else if (UserPrefs.audio_output_device.GetIndex() == -1)
      outputDeviceName = selectedDeviceType->getDeviceNames()[selectedDeviceType->getDefaultDeviceIndex(false)];

   juce::String inputDeviceName;
   if (selectedDeviceType->hasSeparateInputsAndOutputs())
   {
      if (UserPrefs.audio_input_device.GetIndex() >= 0)
         inputDeviceName = selectedDeviceType->getDeviceNames(true)[UserPrefs.audio_input_device.GetIndex()];
      else if (UserPrefs.audio_input_device.GetIndex() == -1)
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
   
   if (toUpdate.empty() || VectorContains(UserPrefs.samplerate.GetDropdown(), toUpdate))
   {
      UserPrefs.samplerate.GetIndex() = -1;
      UserPrefs.samplerate.GetDropdown()->Clear();
      i = 0;
      for (auto rate : selectedDevice->getAvailableSampleRates())
      {
         UserPrefs.samplerate.GetDropdown()->AddLabel(ofToString(rate), i);
         if (rate == gSampleRate)
            UserPrefs.samplerate.GetIndex() = i;
         ++i;
      }
   }

   if (toUpdate.empty() || VectorContains(UserPrefs.buffersize.GetDropdown(), toUpdate))
   {
      UserPrefs.buffersize.GetIndex() = -1;
      UserPrefs.buffersize.GetDropdown()->Clear();
      i = 0;
      for (auto bufferSize : selectedDevice->getAvailableBufferSizes())
      {
         UserPrefs.buffersize.GetDropdown()->AddLabel(ofToString(bufferSize), i);
         if (bufferSize == gBufferSize)
            UserPrefs.buffersize.GetIndex() = i;
         ++i;
      }
   }

   if (selectedDevice != deviceManager.getCurrentAudioDevice())
      delete selectedDevice;
}

void UserPrefsEditor::DrawModule()
{
   auto& deviceManager = TheSynth->GetAudioDeviceManager();
   auto* selectedDeviceType = UserPrefs.devicetype.GetIndex() != -1 ? deviceManager.getAvailableDeviceTypes()[UserPrefs.devicetype.GetIndex()] : deviceManager.getCurrentDeviceTypeObject();

   mCategorySelector->Draw();

   int controlX = 175;
   int controlY = 50;
   bool hasPrefThatRequiresRestart = false;
   for (auto* pref : UserPrefs.mUserPrefs)
   {
      bool onPage = pref->mCategory == mCategory;
      bool hide = false;
      if (pref == &UserPrefs.audio_input_device)
         hide = !selectedDeviceType->hasSeparateInputsAndOutputs();
      if (pref == &UserPrefs.position_x || pref == &UserPrefs.position_y)
         hide = !UserPrefs.set_manual_window_position.Get();

      pref->GetControl()->SetShowing(onPage && !hide);

      if (pref->GetControl()->IsShowing())
      {
         pref->GetControl()->SetPosition(controlX, controlY);
         DrawTextNormal(pref->mName, 3, pref->GetControl()->GetPosition(K(local)).y + 12);
         pref->GetControl()->Draw();

         if (PrefRequiresRestart(pref) && pref->DiffersFromSavedValue())
         {
            DrawRightLabel(pref->GetControl(), "*", ofColor::magenta, 4);
            hasPrefThatRequiresRestart = true;
         }
      }

      if (onPage)
         controlY += 17;
   }
   controlY += 17;
   mSaveButton->SetPosition(controlX, controlY);
   mSaveButton->Draw();
   mCancelButton->SetPosition(mSaveButton->GetRect(K(local)).getMaxX() + 10, controlY);
   mCancelButton->Draw();
   mWidth = 1150;
   mHeight = controlY + 20;

   if (UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex()) == "DirectSound")
      DrawRightLabel(UserPrefs.devicetype.GetControl(), "warning: DirectSound can cause crackle and strange behavior for some sample rates and buffer sizes", ofColor::yellow);

   if (!selectedDeviceType->hasSeparateInputsAndOutputs() && mCategory == UserPrefCategory::General)
   {
      ofRectangle rect = UserPrefs.audio_output_device.GetControl()->GetRect(true);
      ofPushStyle();
      ofSetColor(ofColor::white);
      DrawTextNormal("note: " + UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex()) + " uses the same audio device for output and input", rect.x, rect.getMaxY() + 14, 13);
      ofPopStyle();
   }

   if (UserPrefs.samplerate.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType->hasSeparateInputsAndOutputs())
         DrawRightLabel(UserPrefs.samplerate.GetControl(), "couldn't find a sample rate compatible between these output and input devices", ofColor::yellow);
      else
         DrawRightLabel(UserPrefs.samplerate.GetControl(), "couldn't find any sample rates for this device, for some reason (is it plugged in?)", ofColor::yellow);
   }

   if (UserPrefs.buffersize.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType->hasSeparateInputsAndOutputs())
         DrawRightLabel(UserPrefs.buffersize.GetControl(), "couldn't find a buffer size compatible between these output and input devices", ofColor::yellow);
      else
         DrawRightLabel(UserPrefs.buffersize.GetControl(), "couldn't find any buffer sizes for this device, for some reason (is it plugged in?)", ofColor::yellow);
   }

   DrawRightLabel(UserPrefs.width.GetControl(), "(currently: " + ofToString(ofGetWidth()) + ")", ofColor::white);
   DrawRightLabel(UserPrefs.height.GetControl(), "(currently: " + ofToString(ofGetHeight()) + ")", ofColor::white);
   
   if (UserPrefs.set_manual_window_position.Get())
   {
      auto pos = TheSynth->GetMainComponent()->getTopLevelComponent()->getScreenPosition();
         DrawRightLabel(UserPrefs.position_y.GetControl(), "(currently: " + ofToString(pos.y) + ")", ofColor::white);
         DrawRightLabel(UserPrefs.position_x.GetControl(), "(currently: " + ofToString(pos.x) + ")", ofColor::white);
   }

   DrawRightLabel(UserPrefs.zoom.GetControl(), "(currently: " + ofToString(gDrawScale) + ")", ofColor::white);
   DrawRightLabel(UserPrefs.recordings_path.GetControl(), "(default: " + UserPrefs.recordings_path.GetDefault() + ")", ofColor::white);
   DrawRightLabel(UserPrefs.tooltips.GetControl(), "(default: " + UserPrefs.tooltips.GetDefault() + ")", ofColor::white);
   DrawRightLabel(UserPrefs.layout.GetControl(), "(default: " + UserPrefs.layout.GetDefault() + ")", ofColor::white);
   DrawRightLabel(UserPrefs.youtube_dl_path.GetControl(), "(default: " + UserPrefs.youtube_dl_path.GetDefault() + ")", ofColor::white);
   DrawRightLabel(UserPrefs.ffmpeg_path.GetControl(), "(default: " + UserPrefs.ffmpeg_path.GetDefault() + ")", ofColor::white);

   if (hasPrefThatRequiresRestart)
      DrawRightLabel(mCancelButton, "*requires restart before taking effect", ofColor::magenta, 4);
}

void UserPrefsEditor::DrawRightLabel(IUIControl* control, std::string text, ofColor color, float offsetX)
{
   if (control->IsShowing())
   {
      ofRectangle rect = control->GetRect(true);
      ofPushStyle();
      ofSetColor(color);
      DrawTextNormal(text, rect.getMaxX() + offsetX, rect.getMaxY() - 3, 13);
      ofPopStyle();
   }
}

void UserPrefsEditor::CleanUpSave(std::string& json)  //remove the markup hack that got the json file to save ordered
{
   for (int i = 0; i < (int)UserPrefs.mUserPrefs.size(); ++i)
      ofStringReplace(json, "**" + UserPrefsHolder::ToStringLeadingZeroes(i) + "**", "", true);
}

bool UserPrefsEditor::PrefRequiresRestart(UserPref* pref) const
{
   return pref == &UserPrefs.devicetype ||
          pref == &UserPrefs.audio_output_device ||
          pref == &UserPrefs.audio_input_device ||
          pref == &UserPrefs.samplerate ||
          pref == &UserPrefs.buffersize ||
          pref == &UserPrefs.oversampling ||
          pref == &UserPrefs.max_output_channels ||
          pref == &UserPrefs.max_input_channels ||  
          pref == &UserPrefs.record_buffer_length_minutes ||
          pref == &UserPrefs.show_minimap;
}

void UserPrefsEditor::Save()
{
   //make a copy
   ofxJSONElement prefsFile = UserPrefs.mUserPrefsFile;

   //remove legacy prefs
   prefsFile.removeMember("vstsearchdirs");
   prefsFile.removeMember("youtube-dl_path");

   for (int i = 0; i < (int)UserPrefs.mUserPrefs.size(); ++i)
      UserPrefs.mUserPrefs[i]->Save(i, prefsFile);

   std::string output = prefsFile.getRawString(true);
   CleanUpSave(output);

   juce::File file(TheSynth->GetUserPrefsPath());
   file.create();
   file.replaceWithText(output);

   if (TheSynth->HasFatalError())   //this popup spawned at load due to a bad init setting. in this case, the button says "save and exit"
      juce::JUCEApplicationBase::quit();
}

void UserPrefsEditor::ButtonClicked(ClickButton* button)
{
   if (button == mSaveButton)
   {
      Save();
      SetShowing(false);
   }

   if (button == mCancelButton)
      SetShowing(false);
}

void UserPrefsEditor::CheckboxUpdated(Checkbox* checkbox)
{
}

void UserPrefsEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (!TheSynth->IsLoadingState())
   {
      if (slider == UserPrefs.ui_scale.GetSlider())
         TheSynth->SetUIScale(UserPrefs.ui_scale.Get());
      if (slider == UserPrefs.lissajous_r.GetSlider() || slider == UserPrefs.lissajous_g.GetSlider() || slider == UserPrefs.lissajous_b.GetSlider())
      {
         ModularSynth::sBackgroundLissajousR = UserPrefs.lissajous_r.Get();
         ModularSynth::sBackgroundLissajousG = UserPrefs.lissajous_g.Get();
         ModularSynth::sBackgroundLissajousB = UserPrefs.lissajous_b.Get();
      }
      if (slider == UserPrefs.background_r.GetSlider() || slider == UserPrefs.background_g.GetSlider() || slider == UserPrefs.background_b.GetSlider())
      {
         ModularSynth::sBackgroundR = UserPrefs.background_r.Get();
         ModularSynth::sBackgroundG = UserPrefs.background_g.Get();
         ModularSynth::sBackgroundB = UserPrefs.background_b.Get();
      }
   }
}

void UserPrefsEditor::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void UserPrefsEditor::TextEntryComplete(TextEntry* entry)
{
}

void UserPrefsEditor::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == UserPrefs.devicetype.GetDropdown())
   {
      UpdateDropdowns({ UserPrefs.audio_output_device.GetDropdown(), UserPrefs.audio_input_device.GetDropdown(), UserPrefs.samplerate.GetDropdown(), UserPrefs.buffersize.GetDropdown() });
   }

   if (list == UserPrefs.audio_output_device.GetDropdown())
   {
      UpdateDropdowns({ UserPrefs.samplerate.GetDropdown(), UserPrefs.buffersize.GetDropdown() });
   }

   if (list == UserPrefs.audio_input_device.GetDropdown())
   {
      UpdateDropdowns({ UserPrefs.samplerate.GetDropdown(), UserPrefs.buffersize.GetDropdown() });
   }
}

void UserPrefsEditor::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

std::vector<IUIControl*> UserPrefsEditor::ControlsToNotSetDuringLoadState() const
{
   return GetUIControls();
}

std::vector<IUIControl*> UserPrefsEditor::ControlsToIgnoreInSaveState() const
{
   return GetUIControls();
}
