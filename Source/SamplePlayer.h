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

#include <iostream>
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

#include "juce_osc/juce_osc.h"

class Sample;

class SamplePlayer : public IAudioProcessor, public IDrawableModule, public INoteReceiver, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public ITextEntryListener, private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
   SamplePlayer();
   ~SamplePlayer();
   static IDrawableModule* Create() { return new SamplePlayer(); }
   
   
   void CreateUIControls() override;
   void Init() override;
   void Poll() override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   bool IsResizable() const override { return true; }
   void Resize(float width, float height) override { mWidth = ofClamp(width, 210, 9999); mHeight = ofClamp(height, 125, 9999); }
   
   void SetCuePoint(int pitch, float startSeconds, float lengthSeconds, float speed);
   void FillData(std::vector<float> data);
   ChannelBuffer* GetCueSampleData(int cueIndex);
   float GetLengthInSeconds() const;
   
   void oscMessageReceived(const juce::OSCMessage& msg) override;
   void oscBundleReceived(const juce::OSCBundle& bundle) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void TextEntryComplete(TextEntry* entry) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;
   
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
   void SwitchAndRamp();
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
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
      
   float mWidth;
   float mHeight;
   
   Sample* mSample;
   bool mOwnsSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float mSpeed;
   float mPlaySpeed;
   float mCuePointSpeed;
   FloatSlider* mSpeedSlider;
   ClickButton* mPlayButton;
   ClickButton* mPauseButton;
   ClickButton* mStopButton;
   bool mPlay;
   bool mLoop;
   bool mRecord;
   Checkbox* mLoopCheckbox;
   Checkbox* mRecordCheckbox;
   bool mScrubbingSample;
   std::string mYoutubeId;
   ClickButton* mDownloadYoutubeButton;
   TextEntry* mDownloadYoutubeSearch;
   char mYoutubeSearch[MAX_TEXTENTRY_LENGTH];
   ClickButton* mLoadFileButton;
   ClickButton* mSaveFileButton;
   bool mIsLoadingSample;
   float mZoomLevel;
   float mZoomOffset;
   ClickButton* mTrimToZoomButton;
   
   bool mOscWheelGrabbed;
   float mOscWheelPos;
   float mOscWheelSpeed;
   
   ChannelBuffer mDrawBuffer;
   
   NoteInputBuffer mNoteInputBuffer;
   ::ADSR mAdsr;
   
   struct SampleCuePoint
   {
      float startSeconds{ 0 };
      float lengthSeconds{ 0 };
      float speed{ 1 };
      bool stopOnNoteOff{ false };
   };
   std::vector<SampleCuePoint> mSampleCuePoints{128};
   DropdownList* mCuePointSelector;
   FloatSlider* mCuePointStartSlider;
   FloatSlider* mCuePointLengthSlider;
   FloatSlider* mCuePointSpeedSlider;
   Checkbox* mCuePointStopCheckbox;
   int mActiveCuePointIndex;
   int mHoveredCuePointIndex;
   bool mSetCuePoint;
   Checkbox* mSetCuePointCheckbox;
   bool mSelectPlayedCuePoint;
   Checkbox* mSelectPlayedCuePointCheckbox;
   int mRecentPlayedCuePoint;
   ClickButton* mPlayCurrentCuePointButton;
   bool mShowGrid;
   Checkbox* mShowGridCheckbox;
   ClickButton* mAutoSlice4;
   ClickButton* mAutoSlice8;
   ClickButton* mAutoSlice16;
   ClickButton* mAutoSlice32;
   ClickButton* mPlayHoveredClipButton;
   ClickButton* mGrabHoveredClipButton;

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
      float lengthSeconds;
      std::string youtubeId;
   };
   RunningProcessType mRunningProcessType;
   juce::ChildProcess* mRunningProcess;
   std::function<void()> mOnRunningProcessComplete;
   std::vector<YoutubeSearchResult> mYoutubeSearchResults;
   std::array<ClickButton*, kMaxYoutubeSearchResults> mSearchResultButtons;

   ChannelBuffer mLastOutputSample;
   ChannelBuffer mSwitchAndRampVal;
   
   std::vector<ChannelBuffer*> mRecordChunks;
   bool mDoRecording;   //separate this out from mRecord to allow setup in main thread before audio thread starts recording
   int mRecordingLength;
   bool mRecordingAppendMode;
   Checkbox* mRecordingAppendModeCheckbox;
   bool mRecordAsClips;
   Checkbox* mRecordAsClipsCheckbox;
   GateEffect mRecordGate;
   int mRecordAsClipsCueIndex;
   bool mStopOnNoteOff{ false };
};

