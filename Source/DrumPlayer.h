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

class SamplePlayer;

class DrumPlayer : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public ITextEntryListener, public IGridControllerListener, public ITimeListener
{
public:
   DrumPlayer();
   ~DrumPlayer();
   static IDrawableModule* Create() { return new DrumPlayer(); }


   void CreateUIControls() override;
   void Init() override;

   void Poll() override;

   static void SetUpHitDirectories();

   void ImportSampleCuePoint(SamplePlayer* player, int sourceCueIndex, int destHitIndex);

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return 1 + (int)mIndividualOutputs.size(); }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;
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
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   struct StoredDrumKit
   {
      std::string mName;
      std::string mSampleFiles[NUM_DRUM_HITS];
      int mLinkIds[NUM_DRUM_HITS]{};
      float mVols[NUM_DRUM_HITS]{};
      float mSpeeds[NUM_DRUM_HITS]{};
      float mPans[NUM_DRUM_HITS]{};
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
   void SetHitSample(int sampleIndex, Sample* sample);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(float x, float y, bool right) override;
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;

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
   std::string mAuditionDir;
   char mNewKitName[MAX_TEXTENTRY_LENGTH];
   TextEntry* mNewKitNameEntry;
   ofMutex mLoadSamplesAudioMutex;
   ofMutex mLoadSamplesDrawMutex;
   bool mLoadingSamples;
   ClickButton* mShuffleButton;
   int mSelectedHitIdx;
   bool mMonoOutput;
   Checkbox* mMonoCheckbox;
   GridControlTarget* mGridControlTarget;
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
      IndividualOutput(DrumPlayer* owner, int hitIndex)
      : mDrumPlayer(owner)
      , mHitIndex(hitIndex)
      , mVizBuffer(nullptr)
      , mPatchCableSource(nullptr)
      {
         mVizBuffer = new RollingBuffer(VIZ_BUFFER_SECONDS * gSampleRate);
         mPatchCableSource = new PatchCableSource(owner, kConnectionType_Audio);

         mPatchCableSource->SetOverrideVizBuffer(mVizBuffer);
         mPatchCableSource->SetManualSide(PatchCableSource::Side::kRight);
         owner->AddPatchCableSource(mPatchCableSource);
      }
      ~IndividualOutput()
      {
         delete mVizBuffer;
      }
      void UpdatePosition(int outputIndex)
      {
         float w, h;
         mDrumPlayer->GetDimensions(w, h);
         mPatchCableSource->SetManualPosition(w, 7 + outputIndex * 12);
      }
      DrumPlayer* mDrumPlayer;
      int mHitIndex;
      RollingBuffer* mVizBuffer;
      PatchCableSource* mPatchCableSource;
   };

   std::vector<IndividualOutput*> mIndividualOutputs;

   struct DrumHit
   {
      struct Playhead
      {
         double mStartTime{ -1 };
         double mCutOffTime{ -1 };
         double mOffset{ 0 };
         double mEnvelopeTime{ 0 };
         double mEnvelopeScale{ 1 };
         float mSpeedTweak{ 1 };
      };

      DrumHit()
      : mLinkId(-1)
      , mVol(1)
      , mSpeed(1)
      , mVelocity(1)
      , mPanInput(0)
      , mStartOffset(0)
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
      void LoadNextSample(int direction);
      void LoadSample(std::string path);
      void GrabSample();
      void StartPlayhead(double time, float startOffsetPercent, float velocity);
      void StopLinked(double time);
      float GetPlayProgress(double time);

      Sample mSample;
      int mLinkId;
      float mVol;
      float mSpeed;
      float mVelocity;
      float mPanInput;
      float mStartOffset;
      ModulationChain* mPitchBend;

      bool mUseEnvelope;
      ::ADSR mEnvelope;
      float mEnvelopeLength;
      float mPan;
      int mWiden;
      bool mHasIndividualOutput;
      std::string mHitDirectory;
      int mButtonHeldVelocity;

      DrumPlayer* mOwner;
      FloatSlider* mVolSlider{ nullptr };
      FloatSlider* mSpeedSlider{ nullptr };
      ClickButton* mTestButton{ nullptr };
      ClickButton* mRandomButton{ nullptr };
      ClickButton* mNextButton{ nullptr };
      ClickButton* mPrevButton{ nullptr };
      ClickButton* mGrabSampleButton{ nullptr };
      Checkbox* mUseEnvelopeCheckbox{ nullptr };
      ADSRDisplay* mEnvelopeDisplay{ nullptr };
      FloatSlider* mPanSlider{ nullptr };
      IntSlider* mWidenSlider{ nullptr };
      Checkbox* mIndividualOutputCheckbox{ nullptr };
      FloatSlider* mEnvelopeLengthSlider{ nullptr };
      IntSlider* mLinkIdSlider{ nullptr };
      DropdownList* mHitCategoryDropdown{ nullptr };
      FloatSlider* mStartOffsetSlider{ nullptr };
      int mHitCategoryIndex{ -1 };
      std::string mHitCategory;
      RollingBuffer mWidenerBuffer;
      int mSamplesRemainingToProcess;

      std::array<Playhead, 2> mPlayheads;
      int mCurrentPlayheadIndex;
   };

   std::array<DrumHit, NUM_DRUM_HITS> mDrumHits;
};

#endif /* defined(__modularSynth__DrumPlayer__) */
