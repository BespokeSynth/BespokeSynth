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
//
//  VSTPlugin.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/18/16.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "PolyphonyMgr.h"
#include "INoteReceiver.h"
#include "INoteSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Ramp.h"
#include "VSTPlayhead.h"
#include "VSTWindow.h"

#include <atomic>

#include "juce_audio_processors/juce_audio_processors.h"

class ofxJSONElement;
//class NSWindowOverlay;

namespace VSTLookup
{
   void GetAvailableVSTs(std::vector<juce::PluginDescription>& vsts);
   void FillVSTList(DropdownList* list);
   std::string GetVSTPath(std::string vstName);
   bool GetPluginDesc(juce::PluginDescription& desc, juce::String pluginId);
   void SortByLastUsed(std::vector<juce::PluginDescription>& vsts);
   void GetRecentPlugins(std::vector<juce::PluginDescription>& recentPlugins, int num);
}

class VSTPlugin : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public juce::AudioProcessorListener
{
public:
   VSTPlugin();
   virtual ~VSTPlugin() override;
   static IDrawableModule* Create() { return new VSTPlugin(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   std::string GetTitleLabel() const override;
   void CreateUIControls() override;
   void AddExtraOutputCable();
   void RemoveExtraOutputCable();
   void RecreateUIOutputCables();

   void SetVol(float vol) { mVol = vol; }

   void Poll() override;
   void Exit() override;

   juce::AudioProcessor* GetAudioProcessor() { return mPlugin.get(); }

   void SetVST(juce::PluginDescription pluginDesc);
   void OnVSTWindowClosed();

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const juce::MidiMessage& message) override;

   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void OnUIControlRequested(const char* name) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void LoadVST(juce::PluginDescription desc);
   void LoadVSTFromSaveData(FileStreamIn& in, int rev);
   void GetVSTFileDesc(std::string vstName, juce::PluginDescription& desc);

   std::string GetPluginName() const;
   std::string GetPluginFormatName() const;
   std::string GetPluginId() const;
   void CreateParameterSliders();
   void RefreshPresetFiles();

   //juce::AudioProcessorListener
   void audioProcessorParameterChanged(juce::AudioProcessor* processor, int parameterIndex, float newValue) override {}
   void audioProcessorChanged(juce::AudioProcessor* processor, const ChangeDetails& details) override;
   void audioProcessorParameterChangeGestureBegin(juce::AudioProcessor* processor, int parameterIndex) override;

   float mVol{ 1 };
   FloatSlider* mVolSlider{ nullptr };
   int mPresetFileIndex{ -1 };
   DropdownList* mPresetFileSelector{ nullptr };
   bool mPresetFileUpdateQueued{ false };
   ClickButton* mSavePresetFileButton{ nullptr };
   std::vector<std::string> mPresetFilePaths;
   ClickButton* mOpenEditorButton{ nullptr };
   ClickButton* mPanicButton{ nullptr };
   ClickButton* mAddExtraOutputButton{ nullptr };
   ClickButton* mRemoveExtraOutputButton{ nullptr };
   std::atomic<bool> mWantsPanic{ false };
   std::atomic<bool> mWantsAddExtraOutput{ false };
   std::atomic<bool> mWantsRemoveExtraOutput{ false };

   bool mPluginReady{ false };
   std::unique_ptr<juce::AudioProcessor> mPlugin;
   std::string mPluginName;
   std::string mPluginFormatName;
   std::string mPluginId;
   std::unique_ptr<VSTWindow> mWindow;
   juce::MidiBuffer mMidiBuffer;
   juce::MidiBuffer mFutureMidiBuffer;
   juce::CriticalSection mMidiInputLock;
   std::atomic<bool> mRescanParameterNames{ false };
   juce::String cutOffIdHash(juce::String);
   int mNumInputChannels{ 2 };
   int mNumOutputChannels{ 2 };

   int mNumInBuses{ 0 };
   int mNumOutBuses{ 0 };

   struct ParameterSlider
   {
      VSTPlugin* mOwner{ nullptr };
      float mValue{ 0 };
      FloatSlider* mSlider{ nullptr };
      juce::AudioProcessorParameter* mParameter{ nullptr };
      bool mShowing{ false };
      bool mInSelectorList{ true };
      std::string mDisplayName;
      std::string mID;
      void MakeSlider();
   };

   std::vector<ParameterSlider> mParameterSliders;
   int mChangeGestureParameterIndex{ -1 };

   int mChannel{ 1 };
   bool mUseVoiceAsChannel{ false };
   float mPitchBendRange{ 2 };
   int mModwheelCC{ 1 }; //or 74 in Multidimensional Polyphonic Expression (MPE) spec
   std::string mOldVstPath{ "" }; //for loading save files that predate pluginId-style saving
   int mParameterVersion{ 1 };

   // juce supports a max of 16 stereo output channels
   static const int maxStereoOutputChannels{ 16 };

   std::vector <RollingBuffer*> mAdditionalVizBuffers;
   std::vector<AdditionalNoteCable*> mAdditionalOutCables;
   std::vector<PatchCableSource*> mAdditionalOutCableSources;

   struct ChannelModulations
   {
      ModulationParameters mModulation;
      float mLastPitchBend{ 0 };
      float mLastModWheel{ 0 };
      float mLastPressure{ 0 };
   };

   std::vector<ChannelModulations> mChannelModulations;

   ofMutex mVSTMutex;
   VSTPlayhead mPlayhead;

   //NSWindowOverlay* mWindowOverlay{ nullptr };

   enum DisplayMode
   {
      kDisplayMode_Sliders,
      kDisplayMode_PluginOverlay
   };

   DisplayMode mDisplayMode{ DisplayMode::kDisplayMode_Sliders };
   int mShowParameterIndex{ -1 };
   DropdownList* mShowParameterDropdown{ nullptr };
   static constexpr int kMaxParametersInDropdown{ 30 };
   int mTemporarilyDisplayedParamIndex{ -1 };

   /*
    * Midi and MultiOut support
    */
   AdditionalNoteCable* mMidiOutCable{ nullptr };

   bool mWantOpenVstWindow{ false };
};
