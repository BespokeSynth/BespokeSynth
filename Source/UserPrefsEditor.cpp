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
#include "AudioDeviceSettings.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UserPrefs.h"
#include "PatchCable.h"
#include "QwertyToPitchMapping.h"

#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_gui_basics/juce_gui_basics.h"

UserPrefsEditor::UserPrefsEditor()
: IDrawableModule(1150, 50)
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
   for (auto& oversample : oversampleAmounts)
   {
      UserPrefs.oversampling.GetDropdown()->AddLabel(ofToString(oversample), oversample);
      if (UserPrefs.oversampling.Get() == oversample)
         UserPrefs.oversampling.GetIndex() = oversample;
   }

   std::array<int, 3> bitDepths = { 16, 24, 32 };
   for (auto& bitDepth : bitDepths)
   {
      UserPrefs.output_wav_bit_depth.GetDropdown()->AddLabel(ofToString(bitDepth), bitDepth);
      if (UserPrefs.output_wav_bit_depth.Get() == bitDepth)
         UserPrefs.output_wav_bit_depth.GetIndex() = bitDepth;
   }

   UserPrefs.cable_drop_behavior.GetIndex() = 0;
   UserPrefs.cable_drop_behavior.GetDropdown()->AddLabel("show quickspawn", (int)CableDropBehavior::ShowQuickspawn);
   UserPrefs.cable_drop_behavior.GetDropdown()->AddLabel("do nothing", (int)CableDropBehavior::DoNothing);
   UserPrefs.cable_drop_behavior.GetDropdown()->AddLabel("disconnect", (int)CableDropBehavior::DisconnectCable);

   for (int i = 0; i < UserPrefs.cable_drop_behavior.GetDropdown()->GetNumValues(); ++i)
   {
      if (UserPrefs.cable_drop_behavior.GetDropdown()->GetElement(i).mLabel == UserPrefs.cable_drop_behavior.Get())
         UserPrefs.cable_drop_behavior.GetIndex() = i;
   }

   UserPrefs.qwerty_to_pitch_mode.GetDropdown()->AddLabel("Ableton", (int)QwertyToPitchMappingMode::Ableton);
   UserPrefs.qwerty_to_pitch_mode.GetDropdown()->AddLabel("Fruity", (int)QwertyToPitchMappingMode::Fruity);

   for (int i = 0; i < UserPrefs.qwerty_to_pitch_mode.GetDropdown()->GetNumValues(); ++i)
   {
      if (UserPrefs.qwerty_to_pitch_mode.GetDropdown()->GetElement(i).mLabel == UserPrefs.qwerty_to_pitch_mode.Get())
         UserPrefs.qwerty_to_pitch_mode.GetIndex() = i;
   }

   UserPrefs.minimap_corner.GetIndex() = 0;
   UserPrefs.minimap_corner.GetDropdown()->AddLabel("Top right", (int)MinimapCorner::TopRight);
   UserPrefs.minimap_corner.GetDropdown()->AddLabel("Top left", (int)MinimapCorner::TopLeft);
   UserPrefs.minimap_corner.GetDropdown()->AddLabel("Bottom right", (int)MinimapCorner::BottomRight);
   UserPrefs.minimap_corner.GetDropdown()->AddLabel("Bottom left", (int)MinimapCorner::BottomLeft);

   for (int i = 0; i < UserPrefs.minimap_corner.GetDropdown()->GetNumValues(); ++i)
   {
      if (UserPrefs.minimap_corner.GetDropdown()->GetElement(i).mLabel == UserPrefs.minimap_corner.Get())
         UserPrefs.minimap_corner.GetIndex() = i;
   }
}

void UserPrefsEditor::Show()
{
   mAudioApplyError.clear();
   SetPosition(100 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().x, 250 / TheSynth->GetUIScale() - TheSynth->GetDrawOffset().y);

   UpdateDropdowns({});
   SetShowing(true);

   if (TheSynth->HasFatalError())
   {
      mSaveButton->SetLabel("save and exit");
      mCancelButton->SetShowing(false);
   }

   TheSynth->MoveToFront(this);
}

void UserPrefsEditor::CreatePrefsFileIfNonexistent()
{
   UpdateDropdowns({});

   if (!juce::File(TheSynth->GetUserPrefsPath()).existsAsFile())
      Save();
}

juce::AudioIODeviceType* UserPrefsEditor::GetSelectedAudioDeviceType() const
{
   AudioDeviceSelection selection;
   selection.type = UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex());
   const auto selectedTypeName = GetResolvedAudioDeviceTypeName(TheSynth->GetMainComponent(), selection);
   for (auto* deviceType : TheSynth->GetAudioDeviceManager().getAvailableDeviceTypes())
      if (deviceType->getTypeName().toStdString() == selectedTypeName)
         return deviceType;
   return nullptr;
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
         if (deviceType->getTypeName().toStdString() == UserPrefs.devicetype.Get())
            UserPrefs.devicetype.GetIndex() = i;
         ++i;
      }
   }

   auto* selectedDeviceType = GetSelectedAudioDeviceType();
   if (selectedDeviceType == nullptr)
      return;
   selectedDeviceType->scanForDevices();

   if (toUpdate.empty() || VectorContains(UserPrefs.audio_output_device.GetDropdown(), toUpdate))
   {
      UserPrefs.audio_output_device.GetIndex() = -1;
      UserPrefs.audio_output_device.GetDropdown()->Clear();
      UserPrefs.audio_output_device.GetDropdown()->AddLabel("none", -2);
      UserPrefs.audio_output_device.GetDropdown()->AddLabel("auto", -1);
      i = 0;
      for (auto& outputDevice : selectedDeviceType->getDeviceNames())
      {
         UserPrefs.audio_output_device.GetDropdown()->AddLabel(outputDevice.toStdString(), i);
         if (outputDevice.toStdString() == UserPrefs.audio_output_device.Get())
            UserPrefs.audio_output_device.GetIndex() = i;
         ++i;
      }

      if (UserPrefs.audio_output_device.GetIndex() == -1) //update dropdown to match requested value, in case audio system failed to start
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
      for (auto& inputDevice : selectedDeviceType->getDeviceNames(true))
      {
         UserPrefs.audio_input_device.GetDropdown()->AddLabel(inputDevice.toStdString(), i);
         if (inputDevice.toStdString() == UserPrefs.audio_input_device.Get())
            UserPrefs.audio_input_device.GetIndex() = i;
         ++i;
      }

      if (UserPrefs.audio_input_device.GetIndex() < 0) //update dropdown to match requested value, in case audio system failed to start
      {
         for (int j = -2; j < i; ++j)
         {
            if (UserPrefs.audio_input_device.GetDropdown()->GetLabel(j) == UserPrefs.audio_input_device.Get())
               UserPrefs.audio_input_device.GetIndex() = j;
         }
      }
   }

   juce::String outputDeviceName;
   auto outputNames = selectedDeviceType->getDeviceNames();
   auto inputNames = selectedDeviceType->getDeviceNames(true);
   if (UserPrefs.audio_output_device.GetIndex() >= 0 && UserPrefs.audio_output_device.GetIndex() < outputNames.size())
      outputDeviceName = outputNames[UserPrefs.audio_output_device.GetIndex()];
   else if (UserPrefs.audio_output_device.GetIndex() == -1)
   {
      int defaultIndex = selectedDeviceType->getDefaultDeviceIndex(false);
      if (defaultIndex >= 0 && defaultIndex < outputNames.size())
         outputDeviceName = outputNames[defaultIndex];
   }

   juce::String inputDeviceName;
   if (selectedDeviceType->hasSeparateInputsAndOutputs())
   {
      if (UserPrefs.audio_input_device.GetIndex() >= 0 && UserPrefs.audio_input_device.GetIndex() < inputNames.size())
         inputDeviceName = inputNames[UserPrefs.audio_input_device.GetIndex()];
      else if (UserPrefs.audio_input_device.GetIndex() == -1)
      {
         int defaultIndex = selectedDeviceType->getDefaultDeviceIndex(true);
         if (defaultIndex >= 0 && defaultIndex < inputNames.size())
            inputDeviceName = inputNames[defaultIndex];
      }
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
      int selectedRate = ofToInt(UserPrefs.samplerate.GetDropdown()->GetLabel(UserPrefs.samplerate.GetIndex()));
      if (selectedRate <= 0)
         selectedRate = UserPrefs.samplerate.Get();
      UserPrefs.samplerate.GetIndex() = -1;
      UserPrefs.samplerate.GetDropdown()->Clear();
      i = 0;
      auto rates = selectedDevice->getAvailableSampleRates();
      if (selectedRate > 0)
         rates.addIfNotAlreadyThere((double)selectedRate);
      for (auto& rate : rates)
      {
         UserPrefs.samplerate.GetDropdown()->AddLabel(ofToString(rate), i);
         if (rate == selectedRate)
            UserPrefs.samplerate.GetIndex() = i;
         ++i;
      }
   }

   if (toUpdate.empty() || VectorContains(UserPrefs.buffersize.GetDropdown(), toUpdate))
   {
      int selectedBufferSize = ofToInt(UserPrefs.buffersize.GetDropdown()->GetLabel(UserPrefs.buffersize.GetIndex()));
      if (selectedBufferSize <= 0)
         selectedBufferSize = UserPrefs.buffersize.Get();
      UserPrefs.buffersize.GetIndex() = -1;
      UserPrefs.buffersize.GetDropdown()->Clear();
      i = 0;
      auto bufferSizes = selectedDevice->getAvailableBufferSizes();
      if (selectedBufferSize > 0)
         bufferSizes.addIfNotAlreadyThere(selectedBufferSize);
      for (auto& bufferSize : bufferSizes)
      {
         UserPrefs.buffersize.GetDropdown()->AddLabel(ofToString(bufferSize), i);
         if (bufferSize == selectedBufferSize)
            UserPrefs.buffersize.GetIndex() = i;
         ++i;
      }
   }

   if (selectedDevice != deviceManager.getCurrentAudioDevice())
      delete selectedDevice;
}

void UserPrefsEditor::DrawModule()
{
   auto* selectedDeviceType = GetSelectedAudioDeviceType();

   mCategorySelector->Draw();

   int controlX = 205;
   int controlY = 50;
   bool hasPrefThatRequiresRestart = false;
   for (auto* pref : UserPrefs.mUserPrefs)
   {
      bool onPage = pref->mCategory == mCategory;
      bool hide = false;
      if (pref == &UserPrefs.audio_input_device)
         hide = selectedDeviceType == nullptr || !selectedDeviceType->hasSeparateInputsAndOutputs();
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

   if (mCategory == UserPrefCategory::General)
   {
      auto hardware = GetActiveAudioHardwareSettings(TheSynth->GetMainComponent());
      auto engine = GetActiveAudioEngineSettings(TheSynth->GetMainComponent());
      if (hardware.outputSampleRate > 0 || hardware.inputSampleRate > 0)
      {
         std::string format = "running: engine " + ofToString(engine.sampleRate) + " Hz / " + ofToString(engine.bufferSize) + " samples";
         if (hardware.outputSampleRate > 0)
            format += "; output " + ofToString(hardware.outputSampleRate) + " Hz / " + ofToString(hardware.outputBufferSize) + " samples";
         if (hardware.inputSampleRate > 0)
            format += "; input " + ofToString(hardware.inputSampleRate) + " Hz / " + ofToString(hardware.inputBufferSize) + " samples";
         DrawTextNormal(format, 3, mHeight + 12, 11);
         mHeight += 20;
      }
   }

   if (!mAudioApplyError.empty())
   {
      ofPushStyle();
      ofSetColor(ofColor::yellow);
      DrawTextNormal("audio device: " + mAudioApplyError, 3, mHeight + 12, 11);
      ofPopStyle();
      mHeight += 20;
   }
   if (mAudioSettingsPendingRestart && mCategory == UserPrefCategory::General)
   {
      ofPushStyle();
      ofSetColor(ofColor::magenta);
      DrawTextNormal("audio settings saved; restart to apply them", 3, mHeight + 12, 11);
      ofPopStyle();
      mHeight += 20;
   }

   if (UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex()) == "DirectSound")
      DrawRightLabel(UserPrefs.devicetype.GetControl(), "warning: DirectSound can cause crackle and strange behavior for some sample rates and buffer sizes", ofColor::yellow);

   if (selectedDeviceType != nullptr && !selectedDeviceType->hasSeparateInputsAndOutputs() && mCategory == UserPrefCategory::General)
   {
      ofRectangle rect = UserPrefs.audio_output_device.GetControl()->GetRect(true);
      ofPushStyle();
      ofSetColor(ofColor::white);
      DrawTextNormal("note: " + UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex()) + " uses the same audio device for output and input", rect.x, rect.getMaxY() + 14, 11);
      ofPopStyle();
   }

   if (UserPrefs.samplerate.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType != nullptr && selectedDeviceType->hasSeparateInputsAndOutputs())
         DrawRightLabel(UserPrefs.samplerate.GetControl(), "couldn't find a sample rate compatible between these output and input devices", ofColor::yellow);
      else
         DrawRightLabel(UserPrefs.samplerate.GetControl(), "couldn't find any sample rates for this device, for some reason (is it plugged in?)", ofColor::yellow);
   }

   if (UserPrefs.buffersize.GetDropdown()->GetNumValues() == 0)
   {
      if (selectedDeviceType != nullptr && selectedDeviceType->hasSeparateInputsAndOutputs())
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
   DrawRightLabel(UserPrefs.samples_path.GetControl(), "(default: " + UserPrefs.samples_path.GetDefault() + ")", ofColor::white);
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
      DrawTextNormal(text, rect.getMaxX() + offsetX, rect.getMaxY() - 3, 11);
      ofPopStyle();
   }
}

void UserPrefsEditor::CleanUpSave(std::string& json) //remove the markup hack that got the json file to save ordered
{
   for (int i = 0; i < (int)UserPrefs.mUserPrefs.size(); ++i)
      ofStringReplace(json, "**" + UserPrefsHolder::ToStringLeadingZeroes(i) + "**", "", true);
}

bool UserPrefsEditor::PrefRequiresRestart(UserPref* pref) const
{
   return pref == &UserPrefs.samplerate ||
          pref == &UserPrefs.buffersize ||
          pref == &UserPrefs.oversampling ||
          pref == &UserPrefs.max_output_channels ||
          pref == &UserPrefs.max_input_channels ||
          pref == &UserPrefs.record_buffer_length_minutes ||
          pref == &UserPrefs.show_minimap;
}

bool UserPrefsEditor::Save()
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
   if (!file.create() || !file.replaceWithText(output))
      return false;
   UserPrefs.mUserPrefsFile.open(TheSynth->GetUserPrefsPath());

   if (TheSynth->HasFatalError()) //this popup spawned at load due to a bad init setting. in this case, the button says "save and exit"
      juce::JUCEApplicationBase::quit();
   return true;
}

void UserPrefsEditor::ButtonClicked(ClickButton* button, double time)
{
   if (button == mSaveButton)
   {
      if (TheSynth->HasFatalError())
      {
         if (!Save())
            mAudioApplyError = "could not save settings";
         return;
      }

      AudioDeviceSelection selection{ UserPrefs.devicetype.GetDropdown()->GetLabel(UserPrefs.devicetype.GetIndex()),
                                      UserPrefs.audio_input_device.GetDropdown()->GetLabel(UserPrefs.audio_input_device.GetIndex()),
                                      UserPrefs.audio_output_device.GetDropdown()->GetLabel(UserPrefs.audio_output_device.GetIndex()) };
      AudioEngineSettings desiredEngine{ ofToInt(UserPrefs.samplerate.GetDropdown()->GetLabel(UserPrefs.samplerate.GetIndex())),
                                         ofToInt(UserPrefs.buffersize.GetDropdown()->GetLabel(UserPrefs.buffersize.GetIndex())),
                                         ofToInt(UserPrefs.oversampling.GetDropdown()->GetLabel(UserPrefs.oversampling.GetIndex())),
                                         UserPrefs.max_input_channels.Get(),
                                         UserPrefs.max_output_channels.Get() };
      if (desiredEngine.sampleRate <= 0 || desiredEngine.bufferSize <= 0 || desiredEngine.oversampling <= 0)
      {
         mAudioApplyError = "choose a valid sample rate, buffer size, and oversampling amount";
         return;
      }
      if (desiredEngine != GetActiveAudioEngineSettings(TheSynth->GetMainComponent()))
      {
         if (Save())
         {
            UserPrefs.devicetype.Get() = selection.type;
            UserPrefs.audio_input_device.Get() = selection.input;
            UserPrefs.audio_output_device.Get() = selection.output;
            UserPrefs.samplerate.Get() = desiredEngine.sampleRate;
            UserPrefs.buffersize.Get() = desiredEngine.bufferSize;
            mAudioSettingsPendingRestart = true;
            mAudioApplyError.clear();
         }
         else
            mAudioApplyError = "could not save settings";
         return;
      }
      if (selection != GetActiveAudioDeviceSelection(TheSynth->GetMainComponent()))
      {
         auto result = ApplyAudioDeviceSelection(TheSynth->GetMainComponent(), selection);
         if (!result.success)
         {
            mAudioApplyError = result.error;
            return;
         }
      }
      UserPrefs.devicetype.Get() = selection.type;
      UserPrefs.audio_input_device.Get() = selection.input;
      UserPrefs.audio_output_device.Get() = selection.output;
      if (Save())
      {
         mAudioSettingsPendingRestart = false;
         mAudioApplyError.clear();
         SetShowing(false);
      }
      else
         mAudioApplyError = "could not save settings";
   }

   if (button == mCancelButton)
   {
      mAudioApplyError.clear();
      UpdateDropdowns({});
      SetShowing(false);
   }
}

void UserPrefsEditor::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void UserPrefsEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
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
      if (slider == UserPrefs.cable_alpha.GetSlider())
         ModularSynth::sCableAlpha = UserPrefs.cable_alpha.Get();
   }
}

void UserPrefsEditor::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void UserPrefsEditor::TextEntryComplete(TextEntry* entry)
{
}

void UserPrefsEditor::DropdownUpdated(DropdownList* list, int oldVal, double time)
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

   // Update the output_wav_bit_depth value when the dropdown selection changes
   if (list == UserPrefs.output_wav_bit_depth.GetDropdown())
   {
      UserPrefs.output_wav_bit_depth.Get() = ofToInt(UserPrefs.output_wav_bit_depth.GetDropdown()->GetLabel(UserPrefs.output_wav_bit_depth.GetIndex()));
   }
}

void UserPrefsEditor::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
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
