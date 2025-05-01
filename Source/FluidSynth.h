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

#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "ChannelBuffer.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "NamedMutex.h"
#include "Slider.h"
#include "TextEntry.h"

#include <fluidsynth.h>


class FluidSynth : public IAudioSource, public IDrawableModule, public INoteReceiver, public IButtonListener, public IDropdownListener, public IFloatSliderListener, public ITextEntryListener
{
public:
   FluidSynth();
   ~FluidSynth();
   static IDrawableModule* Create() { return new FluidSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void PostLoadState() override;

   //IAudioSource
   void Process(double time) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const juce::MidiMessage& message) override;

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   //ITextEntryListener
   void TextEntryComplete(TextEntry* entry) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   enum class MidiBankSelect
   {
      GS,
      GM,
      XG,
      MMA
   };

   void ClearNotes(bool soundsOff);
   void UpdateVolume();
   void UpdatePresets();
   void PollPresetLocked(int channel);
   void PollAllPresetsLocked();
   void ReloadSoundFont();
   void SetMidiBankSelect();
   void RecreateSynth();


   float mVolume{ .5 };
   FloatSlider* mVolumeSlider{ nullptr };

   std::string mSoundFontPath{};
   TextEntry* mSoundFontPathEntry{ nullptr };

   ClickButton* mSelectSoundFontButton{ nullptr };

   MidiBankSelect mMidiBankSelect{ 0 };
   DropdownList* mMidiBankSelectDropdown{ nullptr };

   std::array<int, kNumVoices> mPresets{ 0 };
   std::array<DropdownList*, kNumVoices> mPresetsDropdown{ nullptr };
   std::array<int, kNumVoices> mNotesActive{ 0 };
   std::array<float, kNumVoices> mLastNotePlayTime{ -1 };

   NoteInputBuffer mNoteInputBuffer;
   ChannelBuffer mWriteBuffer;

   NamedMutex mSynthMutex{};

   fluid_settings_t* mSettings{ nullptr };
   fluid_synth_t* mSynth{ nullptr };
   int mSoundFontId{ FLUID_FAILED };
   std::string mLoadedSoundFontPath{};
   std::string mVersionString{};
};
