/*
  ==============================================================================

    SamplePlayer.h
    Created: 19 Oct 2017 10:10:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "RadioButton.h"

class SampleBank;
class Sample;

class SamplePlayer : public IAudioSource, public IDrawableModule, public INoteReceiver, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public ITextEntryListener, private OSCReceiver, private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
   SamplePlayer();
   ~SamplePlayer();
   static IDrawableModule* Create() { return new SamplePlayer(); }
   
   string GetTitleLabel() override { return "sampleplayer"; }
   void CreateUIControls() override;
   void Init() override;
   void Poll() override;
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   bool IsResizable() const override { return true; }
   void Resize(float width, float height) override { mWidth = ofClamp(width, 210, 9999); mHeight = ofClamp(height, 125, 9999); }
   
   void SetCuePoint(int pitch, float startSeconds, float lengthSeconds, float speed);
   void FillData(vector<float> data);
   ChannelBuffer* GetCueSampleData(int cueIndex);
   
   void oscMessageReceived(const OSCMessage& msg) override;
   void oscBundleReceived(const OSCBundle& bundle) override;
   
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
   vector<IUIControl*> ControlsToIgnoreInSaveState() const override;
   
private:
   void UpdateSample(Sample* sample, bool ownsSample);
   void UpdateSampleList();
   float GetPlayPositionForMouse(float mouseX) const;
   void GetPlayInfoForPitch(int pitch, float& startSeconds, float& lengthSeconds, float& speed) const;
   void DownloadYoutube(string url, string titles);
   void SearchYoutube(string searchTerm);
   void LoadFile();
   void OnYoutubeSearchComplete(string searchTerm, double searchStartTime);
   void OnYoutubeDownloadComplete(string filename, string title);
   void SwitchAndRamp();
   void SetCuePointForX(float mouseX);
   int GetZoomStartSample() const;
   int GetZoomEndSample() const;
   float GetZoomStartSeconds() const;
   float GetZoomEndSeconds() const;
   void UpdateActiveCuePoint();
   void PlayCuePoint(double time, int index, int velocity, float speedMult, float startOffsetSeconds);
   void RunProcess(const StringArray& args);
   void AutoSlice(NoteInterval interval);
   
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
   
   SampleBank* mBank;
   Sample* mSample;
   bool mOwnsSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float mSpeed;
   float mPlaySpeed;
   FloatSlider* mSpeedSlider;
   int mSampleIndex;
   DropdownList* mSampleList;
   ClickButton* mPlayButton;
   ClickButton* mPauseButton;
   ClickButton* mStopButton;
   bool mPlay;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   PatchCableSource* mSampleBankCable;
   bool mScrubbingSample;
   string mYoutubeId;
   ClickButton* mDownloadYoutubeButton;
   TextEntry* mDownloadYoutubeSearch;
   char mYoutubeSearch[MAX_TEXTENTRY_LENGTH];
   ClickButton* mLoadFileButton;
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
      SampleCuePoint() : startSeconds(0), lengthSeconds(0), speed(1) {}
      float startSeconds;
      float lengthSeconds;
      float speed; 
   };
   vector<SampleCuePoint> mSampleCuePoints{128};
   DropdownList* mCuePointSelector;
   FloatSlider* mCuePointStartSlider;
   FloatSlider* mCuePointLengthSlider;
   FloatSlider* mCuePointSpeedSlider;
   int mActiveCuePointIndex;
   bool mSetCuePoint;
   Checkbox* mSetCuePointCheckbox;
   bool mSelectPlayedCuePoint;
   Checkbox* mSelectPlayedCuePointCheckbox;
   int mRecentPlayedCuePoint;
   ClickButton* mPlayCurrentCuePointButton;
   bool mShowGrid;
   Checkbox* mShowGridCheckbox;
   ofRectangle mCueClipGrabRect;
   ClickButton* mAutoSlice4n;
   ClickButton* mAutoSlice8n;
   ClickButton* mAutoSlice16n;

   string mErrorString;

#define kMaxYoutubeSearchResults 10
   enum class RunningProcessType
   {
      None,
      SearchYoutube,
      DownloadYoutube
   };
   struct YoutubeSearchResult
   {
      string name;
      string channel;
      float lengthSeconds;
      string youtubeId;
   };
   RunningProcessType mRunningProcessType;
   ChildProcess* mRunningProcess;
   std::function<void()> mOnRunningProcessComplete;
   vector<YoutubeSearchResult> mYoutubeSearchResults;
   std::array<ClickButton*, kMaxYoutubeSearchResults> mSearchResultButtons;

   ChannelBuffer mLastOutputSample;
   ChannelBuffer mSwitchAndRampVal;
};

