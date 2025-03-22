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

    SamplePlayer.h
    Created: 19 Oct 2017 10:10:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "EnvOscillator.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "RadioButton.h"
#include "GateEffect.h"
#include "IPulseReceiver.h"
#include "SwitchAndRamp.h"

#include "juce_osc/juce_osc.h"

class Sample;

class SamplePlayer : public IAudioProcessor, public IDrawableModule, public INoteReceiver, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public ITextEntryListener, public IPulseReceiver, private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
   SamplePlayer();
   ~SamplePlayer();
   static IDrawableModule* Create() { return new SamplePlayer(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;
   void Poll() override;

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   void OnPulse(double time, float velocity, int flags) override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   bool IsResizable() const override { return true; }
   void Resize(float width, float height) override
   {
      mWidth = ofClamp(width, 210, 9999);
      mHeight = ofClamp(height, 125, 9999);
   }

   void SetCuePoint(int pitch, float startSeconds, float lengthSeconds, float speed);
   void FillData(std::vector<float> data);
   ChannelBuffer* GetCueSampleData(int cueIndex);
   float GetLengthInSeconds() const;

   void oscMessageReceived(const juce::OSCMessage& msg) override;
   void oscBundleReceived(const juce::OSCBundle& bundle) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateSample(Sample* sample, bool ownsSample);
   float GetPlayPositionForMouse(float mouseX) const;
   float GetSecondsForMouse(float mouseX) const;
   void GetPlayInfoForPitch(int pitch, float& startSeconds, float& lengthSeconds, float& speed, bool& stopOnNoteOff) const;
   void DownloadYoutube(std::string url, std::string titles);
   void SearchYoutube(std::string searchTerm);
   void LoadFile();
   void SaveFile();
   void OnYoutubeSearchComplete(std::string searchTerm, double searchStartTime);
   void OnYoutubeDownloadComplete(std::string filename, std::string title);
   void SetCuePointForX(float mouseX);
   int GetZoomStartSample() const;
   int GetZoomEndSample() const;
   float GetZoomStartSeconds() const;
   float GetZoomEndSeconds() const;
   void UpdateActiveCuePoint();
   void PlayCuePoint(double time, int index, int velocity, float speedMult, float startOffsetSeconds);
   void RunProcess(const juce::StringArray& args);
   void AutoSlice(int slices);
   void StopRecording();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

   float mWidth{ 608 };
   float mHeight{ 150 };

   Sample* mSample{ nullptr };
   bool mOwnsSample{ true };

   float mVolume{ 1 };
   FloatSlider* mVolumeSlider{ nullptr };
   float mSpeed{ 1 };
   float mPlaySpeed{ 1 };
   float mCuePointSpeed{ 1 };
   FloatSlider* mSpeedSlider{ nullptr };
   ClickButton* mPlayButton{ nullptr };
   ClickButton* mPauseButton{ nullptr };
   ClickButton* mStopButton{ nullptr };
   bool mPlay{ false };
   bool mLoop{ false };
   bool mRecord{ false };
   Checkbox* mLoopCheckbox{ nullptr };
   Checkbox* mRecordCheckbox{ nullptr };
   bool mScrubbingSample{ false };
   std::string mYoutubeId;
   ClickButton* mDownloadYoutubeButton{ nullptr };
   TextEntry* mDownloadYoutubeSearch{ nullptr };
   char mYoutubeSearch[MAX_TEXTENTRY_LENGTH]{};
   ClickButton* mLoadFileButton{ nullptr };
   ClickButton* mSaveFileButton{ nullptr };
   bool mIsLoadingSample{ false };
   float mZoomLevel{ 1 };
   float mZoomOffset{ 0 };
   ClickButton* mTrimToZoomButton{ nullptr };

   bool mOscWheelGrabbed{ false };
   float mOscWheelPos{ 0 };
   float mOscWheelSpeed{ 0 };

   ChannelBuffer mDrawBuffer{ 0 };

   NoteInputBuffer mNoteInputBuffer;
   ::ADSR mAdsr{ 10, 1, 1, 10 };

   struct SampleCuePoint
   {
      float startSeconds{ 0 };
      float lengthSeconds{ 0 };
      float speed{ 1 };
      bool stopOnNoteOff{ false };
   };
   std::vector<SampleCuePoint> mSampleCuePoints{ 128 };
   DropdownList* mCuePointSelector{ nullptr };
   FloatSlider* mCuePointStartSlider{ nullptr };
   FloatSlider* mCuePointLengthSlider{ nullptr };
   FloatSlider* mCuePointSpeedSlider{ nullptr };
   Checkbox* mCuePointStopCheckbox{ nullptr };
   int mActiveCuePointIndex{ 0 };
   int mHoveredCuePointIndex{ -1 };
   bool mSetCuePoint{ false };
   Checkbox* mSetCuePointCheckbox{ nullptr };
   bool mSelectPlayedCuePoint{ false };
   Checkbox* mSelectPlayedCuePointCheckbox{ nullptr };
   int mRecentPlayedCuePoint{ -1 };
   ClickButton* mPlayCurrentCuePointButton{ nullptr };
   bool mShowGrid{ false };
   Checkbox* mShowGridCheckbox{ nullptr };
   ClickButton* mAutoSlice4{ nullptr };
   ClickButton* mAutoSlice8{ nullptr };
   ClickButton* mAutoSlice16{ nullptr };
   ClickButton* mAutoSlice32{ nullptr };
   ClickButton* mPlayHoveredClipButton{ nullptr };
   ClickButton* mGrabHoveredClipButton{ nullptr };

   std::string mErrorString;

#define kMaxYoutubeSearchResults 10
   enum class RunningProcessType
   {
      None,
      SearchYoutube,
      DownloadYoutube
   };
   struct YoutubeSearchResult
   {
      std::string name;
      std::string channel;
      float lengthSeconds{ 0 };
      std::string youtubeId;
   };
   RunningProcessType mRunningProcessType{ RunningProcessType::None };
   juce::ChildProcess* mRunningProcess{ nullptr };
   std::function<void()> mOnRunningProcessComplete;
   std::vector<YoutubeSearchResult> mYoutubeSearchResults;
   std::array<ClickButton*, kMaxYoutubeSearchResults> mSearchResultButtons{};

   SwitchAndRamp mSwitchAndRamp;

   std::vector<ChannelBuffer*> mRecordChunks;
   bool mDoRecording{ false }; //separate this out from mRecord to allow setup in main thread before audio thread starts recording
   int mRecordingLength{ 0 };
   bool mRecordingAppendMode{ false };
   Checkbox* mRecordingAppendModeCheckbox{ nullptr };
   bool mRecordAsClips{ false };
   Checkbox* mRecordAsClipsCheckbox{ nullptr };
   GateEffect mRecordGate;
   int mRecordAsClipsCueIndex{ 0 };
   bool mStopOnNoteOff{ false };
};
