/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Michael Lotz (contact: mmlr@mlotz.ch)

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
//
//  FluidSynth.cpp
//  modularSynth
//
//  Created by Michael Lotz on 4/5/25.
//
//

#include "FluidSynth.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

#include "juce_gui_basics/juce_gui_basics.h"


#define PRESET(bank, preset) \
   ((bank << 16) | preset)


FluidSynth::FluidSynth()
: mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mSettings = new_fluid_settings();
   fluid_settings_setnum(mSettings, "synth.sample-rate", gSampleRate);
#if DEBUG
   fluid_settings_setint(mSettings, "synth.verbose", 1);
#endif

   char* defaultSoundFont = nullptr;
   fluid_settings_getstr_default(mSettings, "synth.default-soundfont", &defaultSoundFont);
   mSoundFontPath = defaultSoundFont;

   std::stringstream version;
   version << "FluidSynth version " << fluid_version_str();
   mVersionString = version.str();
}

FluidSynth::~FluidSynth()
{
   delete_fluid_synth(mSynth);
   delete_fluid_settings(mSettings);
}

void FluidSynth::Init()
{
   IDrawableModule::Init();
   RecreateSynth();
}

void FluidSynth::PostLoadState()
{
   RecreateSynth();
}

void FluidSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mVolumeSlider, "volume", &mVolume, 0, 10);
   BUTTON(mSelectSoundFontButton, "select soundfont");
   TEXTENTRY(mSoundFontPathEntry, "soundfont", 15, &mSoundFontPath);
   DROPDOWN(mMidiBankSelectDropdown, "midi bank select", (int*)&mMidiBankSelect, 30);

   for (int i = 0; i < kNumVoices; i++)
   {
      std::stringstream label;
      label << "preset " << (i < 9 ? "   " : "") << i + 1;
      DROPDOWN(mPresetsDropdown[i], label.str().c_str(), &mPresets[i], 250);
      mPresetsDropdown[i]->DrawLabel(true);
   }
   ENDUIBLOCK0();

   mSoundFontPathEntry->SetFlexibleWidth(true);
   mSoundFontPathEntry->DrawLabel(true);

   mMidiBankSelectDropdown->AddLabel("gs", (int)MidiBankSelect::GS);
   mMidiBankSelectDropdown->AddLabel("gm", (int)MidiBankSelect::GM);
   mMidiBankSelectDropdown->AddLabel("xg", (int)MidiBankSelect::XG);
   mMidiBankSelectDropdown->AddLabel("mma", (int)MidiBankSelect::MMA);
   mMidiBankSelectDropdown->DrawLabel(true);
}

void FluidSynth::Process(double time)
{
   PROFILER(FluidSynth);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   mNoteInputBuffer.Process(time);

   int bufferSize = target->GetBuffer()->BufferSize();

   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   {
      ScopedMutex mutex(&mSynthMutex, "process()");
      if (mSynth == nullptr)
         return;

      fluid_synth_write_float(mSynth, bufferSize, mWriteBuffer.GetChannel(0), 0, 1, mWriteBuffer.GetChannel(1), 0, 1);
   }

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), bufferSize);
   }
}

void FluidSynth::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   ClearNotes(false);
}

void FluidSynth::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      ClearNotes(true);
   }
}

void FluidSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();

   mSelectSoundFontButton->Draw();

   mSoundFontPathEntry->Draw();
   if (mSoundFontId == FLUID_FAILED)
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(255, 0, 0, 75);
      ofSetLineWidth(2);
      ofRectangle rect = mSoundFontPathEntry->GetRect(K(local));
      ofRect(rect);
      ofPopStyle();
   }

   mMidiBankSelectDropdown->Draw();

   for (int i = 0; i < kNumVoices; i++)
   {
      mPresetsDropdown[i]->Draw();
   }

#if 1
   float width;
   float height;
   mSelectSoundFontButton->GetDimensions(width, height);
   auto position = mSelectSoundFontButton->GetPosition(true);
   DrawTextNormal(mVersionString, position.x + width + 15, position.y);
#endif

   const float kHighlightDurationMs = 250;

   ofPushStyle();
   ofNoFill();
   ofSetColor(100, 255, 0, 75);

   for (int i = 0; i < kNumVoices; i++)
   {
      if (mLastNotePlayTime[i] < 0)
         continue;

      float fade = 1;
      float delta = gTime - mLastNotePlayTime[i];
      if (mNotesActive[i] == 0)
      {
         if (delta < 0 || delta >= kHighlightDurationMs)
            continue;

         fade = (1 - delta / kHighlightDurationMs);
      }

      ofSetLineWidth(2 * fade);
      ofRectangle rect = mPresetsDropdown[i]->GetRect(K(local));
      ofRect(rect);
   }

   ofPopStyle();
}

void FluidSynth::GetModuleDimensions(float& width, float& height)
{
   mSoundFontPathEntry->GetDimensions(width, height);

   float dimension;
   for (int i = 0; i < kNumVoices; i++)
   {
      mPresetsDropdown[i]->GetDimensions(dimension, height);
      width = std::max(width, dimension);
   }

   width += 4;
   height = (7 + kNumVoices) * height;
}

void FluidSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FluidSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void FluidSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (TheSynth->IsLoadingState())
      return;

   if (list == mMidiBankSelectDropdown)
   {
      RecreateSynth();
   }
   else
   {
      UpdatePresets();
   }
}

void FluidSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (TheSynth->IsLoadingState())
      return;

   if (slider == mVolumeSlider)
   {
      UpdateVolume();
   }
}

void FluidSynth::TextEntryComplete(TextEntry* entry)
{
   if (TheSynth->IsLoadingState())
      return;

   if (entry == mSoundFontPathEntry)
   {
      ReloadSoundFont();
   }
}

void FluidSynth::ButtonClicked(ClickButton* button, double time)
{
   if (button == mSelectSoundFontButton)
   {
      juce::FileChooser chooser("Select SoundFont", juce::File(mSoundFontPath), "*.sf2;*.sf3", true, false, TheSynth->GetFileChooserParent());
      if (chooser.browseForFileToOpen())
      {
         mSoundFontPathEntry->SetText(chooser.getResult().getFullPathName().toStdString());
         ReloadSoundFont();
      }
   }
}

void FluidSynth::ClearNotes(bool soundsOff)
{
   for (int i = 0; i < kNumVoices; i++)
   {
      mNotesActive[i] = 0;
      mLastNotePlayTime[i] = -1;
   }

   ScopedMutex mutex(&mSynthMutex, "setEnabled()");
   if (mSynth == nullptr)
      return;

   if (soundsOff)
   {
      fluid_synth_all_sounds_off(mSynth, -1);
   }
   else
   {
      fluid_synth_all_notes_off(mSynth, -1);
   }
}

void FluidSynth::UpdateVolume()
{
   fluid_settings_setnum(mSettings, "synth.gain", mVolume);
}

void FluidSynth::UpdatePresets()
{
   ScopedMutex mutex(&mSynthMutex, "updatePreset()");
   if (mSynth == nullptr || mSoundFontId == FLUID_FAILED)
      return;

   for (int i = 0; i < kNumVoices; i++)
   {
      int bank = mPresets[i] >> 16;
      int preset = mPresets[i] & 0x7f;
      fluid_synth_program_select(mSynth, i, mSoundFontId, bank, preset);
   }
}

void FluidSynth::PollPresetLocked(int channel)
{
   fluid_preset_t* preset = fluid_synth_get_channel_preset(mSynth, channel);
   if (preset == nullptr)
      return;

   int bank = fluid_preset_get_banknum(preset);
   int num = fluid_preset_get_num(preset);

   ofLog() << "poll preset " << channel << " bank " << bank << " preset " << num;
   mPresets[channel] = PRESET(bank, num);
   mPresetsDropdown[channel]->Poll();
}

void FluidSynth::ReloadSoundFont()
{
   ScopedMutex mutex(&mSynthMutex, "reloadSoundFont()");
   if (mSynth == nullptr || mSoundFontPath == mLoadedSoundFontPath)
      return;

   if (mSoundFontId != FLUID_FAILED)
   {
      fluid_synth_sfunload(mSynth, mSoundFontId, true);
      mSoundFontId = FLUID_FAILED;
      ClearNotes(false);
   }

   if (mSoundFontPath.empty() || !juce::File(mSoundFontPath).existsAsFile())
      return;

   mLoadedSoundFontPath = mSoundFontPath;
   mSoundFontId = fluid_synth_sfload(mSynth, mSoundFontPath.c_str(), true);
   if (mSoundFontId == FLUID_FAILED)
   {
      ofLog() << "loading soundfont failed: " << mSoundFontPath;
      return;
   }

   fluid_sfont_t* font = fluid_synth_get_sfont_by_id(mSynth, mSoundFontId);

   for (int i = 0; i < kNumVoices; i++)
   {
      mPresetsDropdown[i]->Clear();
   }

   fluid_sfont_iteration_start(font);
   for (fluid_preset_t* preset = fluid_sfont_iteration_next(font);
        preset != nullptr; preset = fluid_sfont_iteration_next(font))
   {
      int bank = fluid_preset_get_banknum(preset);
      int num = fluid_preset_get_num(preset);

      std::stringstream label;
      label << bank + 1 << "/" << num + 1 << ": " << fluid_preset_get_name(preset);

      for (int i = 0; i < kNumVoices; i++)
      {
         mPresetsDropdown[i]->AddLabel(label.str(), PRESET(bank, num));
      }
   }

   for (int i = 0; i < kNumVoices; i++)
   {
      PollPresetLocked(i);
   }
}

void FluidSynth::SetMidiBankSelect()
{
   const char* mode = nullptr;
   switch (mMidiBankSelect)
   {
      case MidiBankSelect::GS:
         mode = "gs";
         break;
      case MidiBankSelect::GM:
         mode = "gm";
         break;
      case MidiBankSelect::XG:
         mode = "xg";
         break;
      case MidiBankSelect::MMA:
         mode = "mma";
         break;
      default:
         return;
   }

   fluid_settings_setstr(mSettings, "synth.midi-bank-select", mode);
}

void FluidSynth::RecreateSynth()
{
   {
      ScopedMutex mutex(&mSynthMutex, "recreateSynth()");
      if (mSynth != nullptr)
      {
         delete_fluid_synth(mSynth);
         mSoundFontId = FLUID_FAILED;
         mLoadedSoundFontPath.clear();
         mSynth = nullptr;
      }

      UpdateVolume();
      SetMidiBankSelect();

      mSynth = new_fluid_synth(mSettings);
   }

   ReloadSoundFont();
   UpdatePresets();
}

void FluidSynth::PlayNote(NoteMessage note)
{
   if (!mEnabled || GetTarget() == nullptr)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time))
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   ofLog() << "fluidsynth playnote: " << note.voiceIdx << ", " << note.velocity << ", " << note.pitch;
   if (note.voiceIdx < 0 || note.voiceIdx > kNumVoices)
      note.voiceIdx = 0;

   ScopedMutex mutex(&mSynthMutex, "playNote()");
   if (mSynth == nullptr)
      return;

   if (note.modulation.pitchBend != nullptr)
   {
      float bend = note.modulation.pitchBend->GetValue(0);
      fluid_synth_pitch_bend(mSynth, note.voiceIdx, (int)(ofMap(bend, -2, 2, 0, 16383, K(clamp)) + 0.5));
   }

   if (note.modulation.modWheel != nullptr)
   {
      float modWheel = note.modulation.modWheel->GetValue(0);
      fluid_synth_cc(mSynth, note.voiceIdx, 1, modWheel * 127);
   }

#if 0
   if (note.modulation.pressure != nullptr)
   {
      float pressure = note.modulation.pressure->GetValue(0);
      fluid_synth_channel_pressure(mSynth, note.voiceIdx, (int)ofMap(pressure, 0, 1, 0, 127, K(clamp)));
   }
#endif

   fluid_synth_noteon(mSynth, note.voiceIdx, note.pitch, note.velocity);
   mNotesActive[note.voiceIdx] = std::max(mNotesActive[note.voiceIdx] + (note.velocity > 0 ? 1 : -1), 0);
   mLastNotePlayTime[note.voiceIdx] = gTime;
}

void FluidSynth::SendCC(int control, int value, int voiceIdx)
{
   if (!mEnabled || GetTarget() == nullptr)
      return;

   ofLog() << "fluidsynth cc";
   if (voiceIdx < 0 || voiceIdx > kNumVoices)
      voiceIdx = 0;

   ScopedMutex mutex(&mSynthMutex, "sendCC()");
   if (mSynth == nullptr)
      return;

   fluid_synth_cc(mSynth, voiceIdx, control, value);
}

void FluidSynth::SendMidi(const juce::MidiMessage& message)
{
   if (!mEnabled || GetTarget() == nullptr)
      return;

   if (message.isNoteOnOrOff())
   {
      // Handled by PlayNote()
      return;
   }

   if (message.isAftertouch() || message.isMidiClock() || message.isMidiStart() || message.isMidiStop() || message.isMidiContinue() || message.isSongPositionPointer() || message.isQuarterFrame())
   {
      // Unused or unhandled
      return;
   }

   ScopedMutex mutex(&mSynthMutex, "sendMidi()");
   if (mSynth == nullptr)
      return;

   if (message.isProgramChange())
   {
      int channel = message.getChannel() - 1;
      fluid_synth_program_change(mSynth, channel, message.getProgramChangeNumber());
      PollPresetLocked(channel);
   }
   else if (message.isPitchWheel())
   {
      fluid_synth_pitch_bend(mSynth, message.getChannel() - 1, message.getPitchWheelValue());
   }
   else if (message.isChannelPressure())
   {
      fluid_synth_channel_pressure(mSynth, message.getChannel() - 1, message.getChannelPressureValue());
   }
   else if (message.isAllNotesOff())
   {
      ClearNotes(false);
   }
   else if (message.isAllSoundOff())
   {
      ClearNotes(true);
   }
   else if (message.isSysEx())
   {
      fluid_synth_sysex(mSynth, reinterpret_cast<const char*>(message.getSysExData()), message.getSysExDataSize(), nullptr, nullptr, nullptr, false);
   }
   else if (message.isController())
   {
      int channel = message.getChannel() - 1;
      fluid_synth_cc(mSynth, channel, message.getControllerNumber(), message.getControllerValue());
   }
}
