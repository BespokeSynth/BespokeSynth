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
   void GetAvailableVSTs(std::vector<std::string>& vsts);
   void FillVSTList(DropdownList* list);
   std::string GetVSTPath(std::string vstName);
   void SortByLastUsed(std::vector<std::string>& vsts);
}

class VSTPlugin : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener
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
   
   void SetVST(std::string vstName);
   void OnVSTWindowClosed();
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   static juce::AudioPluginFormatManager sFormatManager;
   static juce::KnownPluginList sPluginList;
   
private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void LoadVST(juce::PluginDescription desc);
   
   std::string GetPluginName() const;
   std::string GetPluginId() const;
   void CreateParameterSliders();
   void RefreshPresetFiles();
   
   float mVol;
   FloatSlider* mVolSlider;
   int mPresetFileIndex;
   DropdownList* mPresetFileSelector;
   ClickButton* mSavePresetFileButton;
   std::vector<std::string> mPresetFilePaths;
   ClickButton* mOpenEditorButton;
   ClickButton* mPanicButton;
   std::atomic<bool> mWantsPanic{false};
   int mOverlayWidth;
   int mOverlayHeight;
   
   bool mPluginReady;
   std::unique_ptr<juce::AudioProcessor> mPlugin;
   std::string mPluginName;
   std::unique_ptr<VSTWindow> mWindow;
   juce::MidiBuffer mMidiBuffer;
   juce::MidiBuffer mFutureMidiBuffer;
   juce::CriticalSection mMidiInputLock;
   int mNumInputs;
   int mNumOutputs;
   
   struct ParameterSlider
   {
      float mValue;
      FloatSlider* mSlider;
      juce::AudioProcessorParameter* mParameter;
      bool mShowing;
      bool mInSelectorList;
   };
   
   std::vector<ParameterSlider> mParameterSliders;
   
   int mChannel;
   bool mUseVoiceAsChannel;
   float mPitchBendRange;
   int mModwheelCC;
   
   struct ChannelModulations
   {
      ModulationParameters mModulation;
      float mLastPitchBend;
      float mLastModWheel;
      float mLastPressure;
   };
   
   std::vector<ChannelModulations> mChannelModulations;
   
   ofMutex mVSTMutex;
   VSTPlayhead mPlayhead;
   
   //NSWindowOverlay* mWindowOverlay;
   
   enum DisplayMode
   {
      kDisplayMode_Sliders,
      kDisplayMode_PluginOverlay
   };
   
   DisplayMode mDisplayMode;
   int mShowParameterIndex;
   DropdownList* mShowParameterDropdown;
   int mTemporarilyDisplayedParamIndex;

   /*
    * Midi and MultiOut support
    */
   AdditionalNoteCable *mMidiOutCable{nullptr};

   bool mWantOpenVstWindow{ false };
};

#endif /* defined(__Bespoke__VSTPlugin__) */
