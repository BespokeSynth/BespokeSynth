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
#include "Checkbox.h"
#if BESPOKE_IOS
#include "IOSPluginViewHost.h"
#include "IOSAUParameterEvents.h"
#endif

#include <atomic>
#include <list>
#include <memory>

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

//how an embedded plugin UI fits the module, for plugins that can't take every size
enum class PluginEmbedSizeMode
{
   ResizeModuleToFit, //ask the plugin for the module's size at the current zoom, and let the module follow the size it takes
   Clip //the module keeps the size it was set to, and the plugin's UI is clipped or padded if it can't take that size
};

class VSTPlugin : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public juce::AudioProcessorListener
{
public:
   VSTPlugin();
   virtual ~VSTPlugin() override;
   static IDrawableModule* Create() { return new VSTPlugin(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   static const int kMaxJuceMidiStereoChannels = 16;

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
   int GetModuleSaveStateRev() const override { return 5; }
   //embedded plugin UIs can be resized, unless the plugin's editor has a fixed size
   bool IsResizable() const override { return mEmbedUI && !EmbeddedSizeIsFixed(); }
   //the editor doesn't exist yet at load time, so this depends only on mEmbedUI, which LoadState reads before the base state
   bool ShouldSaveSize() const override { return mEmbedUI; }
   void Resize(float width, float height) override;
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void LoadVST(juce::PluginDescription desc);
   void UnloadPlugin();
   void OnPluginCreated(std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error, const juce::PluginDescription& desc);
   void BindPendingSliders();
   void ApplyPendingState();
   void ProcessBypass(ChannelBuffer* buffer);
   void UpdateEmbeddedView();
   bool HasEmbeddedView() const;
   ofVec2f GetEmbedWantedSize() const; //from the module size (mWidth/mHeight) when it's been resized, otherwise the plugin's natural size scaled down
   void FitEmbedSizeToEditor();
   void SetModuleSizeForEmbedSize(ofVec2f embedSize);
   void SetEmbeddedSize(ofVec2f size); //and moves the cables if it changed
   bool EmbeddedSizeIsFixed() const; //the plugin's editor can't be resized, so neither can the module
   void UpdateControlsForEmbed(); //an embedded view hides everything but the top row of controls
   bool CreateEmbeddedView(void* nativeParentView);
   void DestroyEmbeddedView();
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
   int mPresetFileLoadCountdown{ 0 };
   ClickButton* mSavePresetFileButton{ nullptr };
   ClickButton* mResetPresetButton{ nullptr };
   std::vector<std::string> mPresetFilePaths;
   std::string mLastLoadedPresetFilePath{};
   ClickButton* mOpenEditorButton{ nullptr };
   ClickButton* mLoadParameterButton{ nullptr };
   ClickButton* mPanicButton{ nullptr };
   ClickButton* mAddExtraOutputButton{ nullptr };
   ClickButton* mRemoveExtraOutputButton{ nullptr };
   std::atomic<bool> mWantsPanic{ false };
   std::atomic<bool> mWantsAddExtraOutput{ false };
   std::atomic<bool> mWantsRemoveExtraOutput{ false };
   std::atomic<bool> mWantLoadParameters{ false };
   std::atomic<bool> mWantRemoveSlider{ false };
   std::atomic<int> mWantRemoveSliderIndex{ -1 };

   std::atomic<bool> mPluginReady{ false };
   std::unique_ptr<juce::AudioProcessor> mPlugin;

   //plugins are created asynchronously (AUv3 requires it). until the instance arrives, state loaded from a save file waits here
   bool mPluginLoading{ false };
   juce::String mLoadingPluginId;
   int mLoadRequestId{ 0 };
   std::shared_ptr<bool> mAliveToken{ std::make_shared<bool>(true) }; //lets a late creation callback know this module is gone
   struct PendingPluginState
   {
      bool mHasState{ false };
      juce::MemoryBlock mState;
      juce::MemoryBlock mProgramState;
      bool mHasExposedParams{ false };
      std::vector<int> mExposedParams;
   };
   PendingPluginState mPendingState;
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
      //heap allocated so its address never moves: sliders, and other modules (e.g. interface) hold pointers to it
      std::shared_ptr<float> mValue{ std::make_shared<float>(0.0f) };
      FloatSlider* mSlider{ nullptr };
      ClickButton* mRemoveButton{ nullptr };
      juce::AudioProcessorParameter* mParameter{ nullptr };
      bool mShowing{ false };
      bool mInSelectorList{ true };
      bool mHasPendingValue{ false }; //value was set before the plugin existed, push it to the parameter once bound
      std::string mDisplayName;
      std::string mID;
      void MakeSlider();
   };

   std::vector<ParameterSlider> mParameterSliders;
   //sliders requested by name (by cables, saved control values, etc) before the plugin exists. bound to real parameters by id once it's
   //created, sharing their value storage so pointers to it stay valid
   std::list<ParameterSlider> mPendingSliders;
   int mChangeGestureParameterIndex{ -1 };

   int mChannel{ 1 };
   bool mUseVoiceAsChannel{ false };
   float mPitchBendRange{ 2 };
   int mModwheelCC{ 1 }; //or 74 in Multidimensional Polyphonic Expression (MPE) spec
   std::string mOldVstPath{ "" }; //for loading save files that predate pluginId-style saving
   int mParameterVersion{ 1 };
   int mLastNumChannels{ -1 };

   // juce supports a max of 16 stereo output channels
   static const int maxStereoOutputChannels{ 16 };

   std::vector<RollingBuffer*> mAdditionalVizBuffers;
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

   int mShowParameterIndex{ -1 };
   DropdownList* mShowParameterDropdown{ nullptr };
   static constexpr int kMaxParametersInDropdown{ 100 };
   int mTemporarilyDisplayedParamIndex{ -1 };

   /*
    * Midi and MultiOut support
    */
   AdditionalNoteCable* mMidiOutCable{ nullptr };

   bool mWantOpenVstWindow{ false };

   //"embed": show the plugin's own UI inside the module on the canvas, instead of in a window. iOS (AUv3) only for now
   bool mEmbedUI{ false };
   Checkbox* mEmbedUICheckbox{ nullptr };
   float mEmbedTop{ 0 }; //where the embedded UI starts, below the module's controls
   ofVec2f mEmbeddedSize{ 0, 0 }; //size of the embedded UI on the canvas, once the plugin's view has arrived
   PluginEmbedSizeMode mSizeMode{ PluginEmbedSizeMode::Clip };
   bool mEmbedUserSized{ false }; //resized by hand, so the module's mWidth/mHeight set the size instead of the plugin's natural size
   ofVec2f mEmbedNaturalSize{ 0, 0 }; //the plugin view's own size, once it has arrived
   bool mIgnoreResize{ false };
   bool mControlsHiddenForEmbed{ false };
   bool mEmbedFitOnCreate{ false };
   ofVec2f mLastEmbedSize{ 0, 0 };
   static constexpr float kDefaultEmbeddedUIScale{ .4f };
   static constexpr float kEmbedBottomMargin{ 12 }; //keeps the resize handle below the plugin's view, which would take its touches
   //the plugin's view is hidden while the canvas or module moves (resizing it every frame is slow and glitchy),
   //and a black square is drawn in its place
   float mLastEmbedScale{ 0 };
   ofVec2f mLastEmbedCanvasPos{ 0, 0 };
   double mLastEmbedMoveTimeMs{ 0 };
#if BESPOKE_IOS
   IOSPluginViewHost::Overlay* mEmbeddedView{ nullptr };
   IOSAUParameterEvents::Listener* mParameterEvents{ nullptr }; //juce doesn't listen for AU parameter events on iOS, so we do
#else
   std::unique_ptr<juce::Component> mEmbeddedHolder; //native view over the canvas that holds and clips the editor
   std::unique_ptr<juce::AudioProcessorEditor> mEmbeddedEditor;
   bool mEmbeddedEditorResizable{ false };
   juce::Point<int> mEmbedEditorRequest{ -1, -1 }; //last size asked of the editor, in pixels
#endif
};
