//
//  DrumPlayer.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#ifndef __modularSynth__DrumPlayer__
#define __modularSynth__DrumPlayer__

#include <iostream>
#include "IAudioSource.h"
#include "Sample.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "Transport.h"
#include "MidiDevice.h"
#include "TextEntry.h"
#include "ADSR.h"
#include "BiquadFilter.h"
#include "ADSRDisplay.h"
#include "PatchCableSource.h"
#include "RollingBuffer.h"
#include "GridController.h"

#define NUM_DRUM_HITS 16

class DrumPlayer : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public ITextEntryListener, public IGridControllerListener, public ITimeListener
{
public:
   DrumPlayer();
   ~DrumPlayer();
   static IDrawableModule* Create() { return new DrumPlayer(); }
   
   string GetTitleLabel() override { return "drumplayer"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   static string GetDrumHitName(int index);
   static void SetUpHitDirectories();
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return 1 + (int)mIndividualOutputs.size(); }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void TextEntryComplete(TextEntry* entry) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:

   struct StoredDrumKit
   {
      string mName;
      string mSampleFiles[NUM_DRUM_HITS];
      int mLinkIds[NUM_DRUM_HITS];
      float mVols[NUM_DRUM_HITS];
      float mSpeeds[NUM_DRUM_HITS];
      float mPans[NUM_DRUM_HITS];
   };

   void LoadKit(int kit);
   int GetAssociatedSampleIndex(int x, int y);
   void ReadKits();
   void SaveKits();
   void CreateKit();
   void ShuffleKit();
   void UpdateVisibleControls();
   int GetIndividualOutputIndex(int hitIndex);
   void UpdateLights();
   void SetUpNewDrumPlayer();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;
   
   ChannelBuffer mOutputBuffer;
   float mSpeed;
   float mSpeedRandomization;
   float mVolume;
   int mLoadedKit;
   FloatSlider* mVolSlider;
   FloatSlider* mSpeedSlider;
   FloatSlider* mSpeedRandomizationSlider;
   DropdownList* mKitSelector;
   bool mEditMode;
   Checkbox* mEditCheckbox;
   std::vector<StoredDrumKit> mKits;
   ClickButton* mSaveButton;
   ClickButton* mNewKitButton;
   int mAuditionSampleIdx;
   float mAuditionInc;
   FloatSlider* mAuditionSlider;
   string mAuditionDir;
   int mAuditionPadIdx;
   char mNewKitName[MAX_TEXTENTRY_LENGTH];
   TextEntry* mNewKitNameEntry;
   ofMutex mLoadSamplesAudioMutex;
   ofMutex mLoadSamplesDrawMutex;
   bool mLoadingSamples;
   ClickButton* mShuffleButton;
   int mSelectedHitIdx;
   bool mMonoOutput;
   Checkbox* mMonoCheckbox;
   GridController* mGridController;
   NoteInputBuffer mNoteInputBuffer;
   bool mNeedSetup;
   bool mNoteRepeat;
   Checkbox* mNoteRepeatCheckbox;
   NoteInterval mQuantizeInterval;
   DropdownList* mQuantizeIntervalSelector;
   
   void LoadSampleLock();
   void LoadSampleUnlock();
   
   struct IndividualOutput
   {
      IndividualOutput(DrumPlayer* owner, int hitIndex, int outputIndex)
      : mHitIndex(hitIndex)
      , mVizBuffer(nullptr)
      , mPatchCableSource(nullptr)
      {
         mVizBuffer = new RollingBuffer(VIZ_BUFFER_SECONDS*gSampleRate);
         mPatchCableSource = new PatchCableSource(owner, kConnectionType_Audio);
         
         mPatchCableSource->SetManualPosition(152, 7 + outputIndex * 12);
         mPatchCableSource->SetOverrideVizBuffer(mVizBuffer);
         mPatchCableSource->SetManualSide(PatchCableSource::Side::kRight);
         owner->AddPatchCableSource(mPatchCableSource);
      }
      ~IndividualOutput()
      {
         delete mVizBuffer;
      }
      int mHitIndex;
      RollingBuffer* mVizBuffer;
      PatchCableSource* mPatchCableSource;
   };
   
   std::vector<IndividualOutput*> mIndividualOutputs;
   
   struct DrumHit
   {
      struct Playhead
      {
         Playhead() : mStartTime(-1) {}
         double mStartTime;
         double mCutOffTime;
         double mOffset;
         double mEnvelopeTime;
         double mEnvelopeScale;
         float mSpeedTweak;
      };

      DrumHit()
      : mLinkId(-1)
      , mVol(1)
      , mSpeed(1)
      , mVelocity(1)
      , mPanInput(0)
      , mPitchBend(nullptr)
      , mUseEnvelope(false)
      , mEnvelopeLength(200)
      , mPan(0)
      , mWiden(0)
      , mHasIndividualOutput(false)
      , mOwner(nullptr)
      , mWidenerBuffer(2048)
      , mSamplesRemainingToProcess(0)
      , mCurrentPlayheadIndex(0)
      , mButtonHeldVelocity(0)
      {
         mEnvelope.GetHasSustainStage() = false;
         mEnvelope.GetA() = 1;
         mEnvelope.GetD() = 1;
         mEnvelope.GetS() = 1;
         mEnvelope.GetR() = 100;
         mEnvelope.Start(0, 1);
      }
      
      void CreateUIControls(DrumPlayer* owner, int index);
      bool Process(double time, float speed, float vol, ChannelBuffer* out, int bufferSize);
      void SetUIControlsShowing(bool showing);
      void DrawUIControls();
      void UpdateHitDirectoryDropdown();
      void LoadRandomSample();
      void StartPlayhead(double time, float startOffsetPercent, float velocity);
      void StopLinked(double time);
      float GetPlayProgress(double time);
      
      Sample mSample;
      int mLinkId;
      float mVol;
      float mSpeed;
      float mVelocity;
      float mPanInput;
      ModulationChain* mPitchBend;
      
      bool mUseEnvelope;
      ::ADSR mEnvelope;
      float mEnvelopeLength;
      float mPan;
      int mWiden;
      bool mHasIndividualOutput;
      string mHitDirectory;
      int mButtonHeldVelocity;
      
      DrumPlayer* mOwner;
      FloatSlider* mVolSlider;
      FloatSlider* mSpeedSlider;
      ClickButton* mTestButton;
      ClickButton* mRandomButton;
      Checkbox* mUseEnvelopeCheckbox;
      ADSRDisplay* mEnvelopeDisplay;
      FloatSlider* mPanSlider;
      IntSlider* mWidenSlider;
      Checkbox* mIndividualOutputCheckbox;
      FloatSlider* mEnvelopeLengthSlider;
      IntSlider* mLinkIdSlider;
      DropdownList* mHitCategoryDropdown;
      int mHitCategoryIndex;
      string mHitCategory;
      RollingBuffer mWidenerBuffer;
      int mSamplesRemainingToProcess;

      array<Playhead,2> mPlayheads;
      int mCurrentPlayheadIndex;
   };
   
   std::array<DrumHit, NUM_DRUM_HITS> mDrumHits;
};

#endif /* defined(__modularSynth__DrumPlayer__) */

