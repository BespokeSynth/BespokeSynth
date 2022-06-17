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

#ifndef __Bespoke__VSTPlugin__
#define __Bespoke__VSTPlugin__

#include "IAudioProcessor.h"
#include "PolyphonyMgr.h"
#include "INoteReceiver.h"
#include "INoteSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "ClickButton.h"
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
   juce::PluginDescription GetPluginDesc(int id);
   juce::PluginDescription GetPluginDesc(std::string vstName);
   juce::PluginDescription GetPluginDesc(juce::String pluginId);
   void SortByLastUsed(std::vector<std::string>& vsts);
   std::vector<juce::PluginDescription> GetRecentPlugins(int num);
   //juce::PluginDescription stump{};
}

class VSTPlugin : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public juce::AudioProcessorListener
{
public:
   VSTPlugin();
   virtual ~VSTPlugin() override;
   static IDrawableModule* Create() { return new VSTPlugin(); }

   std::string GetTitleLabel() const override;
   void CreateUIControls() override;

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
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const juce::MidiMessage& message) override;

   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;

   void OnUIControlRequested(const char* name) override;
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void LoadVST(juce::PluginDescription desc);
   void LoadVSTFromSaveData(FileStreamIn& in, int rev);
   void GetVSTFileDesc(std::string vstName, juce::PluginDescription& desc);

   std::string GetPluginName() const;
   std::string GetPluginFormatName() const;
   std::string GetPluginId() const;
   void CreateParameterSliders();
   void RefreshPresetFiles();
   bool ParameterNameExists(std::string name, int checkUntilIndex) const;

   //juce::AudioProcessorListener
   void audioProcessorParameterChanged(juce::AudioProcessor* processor, int parameterIndex, float newValue) override {}
   void audioProcessorChanged(juce::AudioProcessor* processor, const ChangeDetails& details) override {}
   void audioProcessorParameterChangeGestureBegin(juce::AudioProcessor* processor, int parameterIndex) override;

   float mVol{ 1 };
   FloatSlider* mVolSlider{ nullptr };
   int mPresetFileIndex{ -1 };
   DropdownList* mPresetFileSelector{ nullptr };
   ClickButton* mSavePresetFileButton{ nullptr };
   std::vector<std::string> mPresetFilePaths;
   ClickButton* mOpenEditorButton{ nullptr };
   ClickButton* mPanicButton{ nullptr };
   std::atomic<bool> mWantsPanic{ false };

   bool mPluginReady{ false };
   std::unique_ptr<juce::AudioProcessor> mPlugin;
   std::string mPluginName;
   std::unique_ptr<VSTWindow> mWindow;
   juce::MidiBuffer mMidiBuffer;
   juce::MidiBuffer mFutureMidiBuffer;
   juce::CriticalSection mMidiInputLock;
   int mNumInputs{ 2 };
   int mNumOutputs{ 2 };

   struct ParameterSlider
   {
      float mValue{ 0 };
      FloatSlider* mSlider{ nullptr };
      juce::AudioProcessorParameter* mParameter{ nullptr };
      bool mShowing{ false };
      bool mInSelectorList{ true };
      std::string mName;
      void MakeSlider(VSTPlugin* owner);
   };

   std::vector<ParameterSlider> mParameterSliders;
   int mChangeGestureParameterIndex{ -1 };

   int mChannel{ 1 };
   bool mUseVoiceAsChannel{ false };
   float mPitchBendRange{ 2 };
   int mModwheelCC{ 1 }; //or 74 in Multidimensional Polyphonic Expression (MPE) spec

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
   int mTemporarilyDisplayedParamIndex{ -1 };

   /*
    * Midi and MultiOut support
    */
   AdditionalNoteCable* mMidiOutCable{ nullptr };

   bool mWantOpenVstWindow{ false };
};

#endif /* defined(__Bespoke__VSTPlugin__) */
