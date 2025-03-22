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

#pragma once

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
#include "Push2Control.h"

#define NUM_DRUM_HITS 16

class SamplePlayer;

class DrumPlayer : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public ITextEntryListener, public IGridControllerListener, public ITimeListener, public IPush2GridController
{
public:
   DrumPlayer();
   ~DrumPlayer();
   static IDrawableModule* Create() { return new DrumPlayer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

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
   void PlayNote(NoteMessage note) override;
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

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;
   bool HasPush2OverrideControls() const override { return mPush2SelectedHitIdx != -1; }
   void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

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
   void OnClicked(float x, float y, bool right) override;
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;

   ChannelBuffer mOutputBuffer;
   float mSpeed{ 1 };
   float mSpeedRandomization{ 0 };
   float mVolume{ 1 };
   int mLoadedKit{ 0 };
   FloatSlider* mVolSlider{ nullptr };
   FloatSlider* mSpeedSlider{ nullptr };
   FloatSlider* mSpeedRandomizationSlider{ nullptr };
   DropdownList* mKitSelector{ nullptr };
   bool mEditMode{ false };
   Checkbox* mEditCheckbox{ nullptr };
   std::vector<StoredDrumKit> mKits;
   ClickButton* mSaveButton{ nullptr };
   ClickButton* mNewKitButton{ nullptr };
   int mAuditionSampleIdx{ 0 };
   float mAuditionInc{ 0 };
   FloatSlider* mAuditionSlider{ nullptr };
   std::string mAuditionDir;
   char mNewKitName[MAX_TEXTENTRY_LENGTH]{};
   TextEntry* mNewKitNameEntry{ nullptr };
   ofMutex mLoadSamplesAudioMutex;
   ofMutex mLoadSamplesDrawMutex;
   bool mLoadingSamples{ false };
   ClickButton* mShuffleButton{ nullptr };
   int mSelectedHitIdx{ 0 };
   bool mMonoOutput{ false };
   Checkbox* mMonoCheckbox{ nullptr };
   bool mSingleVoice{ false };
   Checkbox* mSingleVoiceCheckbox{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
   NoteInputBuffer mNoteInputBuffer{ nullptr };
   bool mNeedSetup{ true };
   bool mNoteRepeat{ false };
   Checkbox* mNoteRepeatCheckbox{ nullptr };
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_None };
   DropdownList* mQuantizeIntervalSelector{ nullptr };
   bool mFullVelocity{ false };
   Checkbox* mFullVelocityCheckbox{ nullptr };
   int mPush2SelectedHitIdx{ -1 };

   void LoadSampleLock();
   void LoadSampleUnlock();

   struct IndividualOutput
   {
      IndividualOutput(DrumPlayer* owner, int hitIndex)
      : mDrumPlayer(owner)
      , mHitIndex(hitIndex)
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
      DrumPlayer* mDrumPlayer{ nullptr };
      int mHitIndex{ 0 };
      RollingBuffer* mVizBuffer{ nullptr };
      PatchCableSource* mPatchCableSource{ nullptr };
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
      {
         mEnvelope.GetHasSustainStage() = false;
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
      int mLinkId{ -1 };
      float mVol{ 1 };
      float mSpeed{ 1 };
      float mVelocity{ 1 };
      float mPanInput{ 0 };
      float mStartOffset{ 0 };
      ModulationChain* mPitchBend{ nullptr };

      bool mUseEnvelope{ false };
      ::ADSR mEnvelope{ 1, 1, 1, 100 };
      float mEnvelopeLength{ 200 };
      float mPan{ 0 };
      int mWiden{ 0 };
      bool mHasIndividualOutput{ false };
      std::string mHitDirectory;
      int mButtonHeldVelocity{ 0 };

      DrumPlayer* mOwner{ nullptr };
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
      RollingBuffer mWidenerBuffer{ 2048 };
      int mSamplesRemainingToProcess{ 0 };

      std::array<Playhead, 2> mPlayheads;
      int mCurrentPlayheadIndex{ 0 };
   };

   std::array<DrumHit, NUM_DRUM_HITS> mDrumHits;
};
