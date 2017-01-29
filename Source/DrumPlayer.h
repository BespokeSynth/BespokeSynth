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

class LooperRecorder;

#define NUM_DRUM_HITS 16

class DrumPlayer : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public ITextEntryListener
{
public:
   DrumPlayer();
   ~DrumPlayer();
   static IDrawableModule* Create() { return new DrumPlayer(); }
   
   string GetTitleLabel() override { return "drumplayer"; }
   void CreateUIControls() override;
   
   static string GetDrumHitName(int index);
   
   void SetLooperRecorder(LooperRecorder* recorder) { mLooperRecorder = recorder; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;

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
   };

   void LoadKit(int kit);
   int GetAssociatedSampleIndex(int x, int y);
   void ReadKits();
   void SaveKits();
   void CreateKit();
   void ShuffleSpeeds();
   void UpdateVisibleControls();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   float* mOutputBuffer;
   float mSpeed;
   float mVolume;
   bool mRecordDrums;
   Checkbox* mRecordDrumsCheckbox;
   int mLoadedKit;
   LooperRecorder* mLooperRecorder;
   FloatSlider* mVolSlider;
   FloatSlider* mSpeedSlider;
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
   ofMutex mLoadSamplesMutex;
   bool mLoadingSamples;
   ClickButton* mShuffleSpeedsButton;
   int mSelectedHitIdx;
   
   struct DrumHit
   {
      DrumHit()
      : mLinkId(0)
      , mVol(1)
      , mSpeed(1)
      , mVelocity(1)
      , mUseEnvelope(false)
      , mUseFilter(false)
      , mCutoff(10000)
      , mQ(1)
      {
      }
      
      void CreateUIControls(DrumPlayer* owner, int index);
      void Process(double time, float speed, float vol, float* out, int bufferSize);
      void SetUIControlsShowing(bool showing);
      void DrawUIControls();
      
      Sample mSample;
      int mLinkId;
      float mVol;
      float mSpeed;
      float mVelocity;
      
      bool mUseEnvelope;
      ADSR mEnvelope;
      bool mUseFilter;
      ADSR mFilterADSR;
      float mCutoff;
      float mQ;
      BiquadFilter mFilter;
      
      FloatSlider* mVolSlider;
      FloatSlider* mSpeedSlider;
      ClickButton* mTestButton;
      Checkbox* mUseEnvelopeCheckbox;
      ADSRDisplay* mEnvelopeDisplay;
      Checkbox* mUseFilterCheckbox;
      ADSRDisplay* mFilterDisplay;
      FloatSlider* mCutoffSlider;
      FloatSlider* mQSlider;
   };
   
   DrumHit mDrumHits[NUM_DRUM_HITS];
};

#endif /* defined(__modularSynth__DrumPlayer__) */

