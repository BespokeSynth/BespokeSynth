//
//  ChaosEngine.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/18/13.
//
//

#ifndef __modularSynth__ChaosEngine__
#define __modularSynth__ChaosEngine__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "ofxJSONElement.h"
#include "Scale.h"
#include "RadioButton.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Chord.h"

class ChaosEngine;

extern ChaosEngine* TheChaosEngine;

class ChaosEngine : public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public INoteSource, public IIntSliderListener
{
public:
   ChaosEngine();
   ~ChaosEngine();
   static IDrawableModule* Create() { return new ChaosEngine(); }
   static bool CanCreate() { return TheChaosEngine == nullptr; }
   
   string GetTitleLabel() override { return "CHAOS ENGINE"; }
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   void AudioUpdate();
   void RestartProgression() { mRestarting = true; mChordProgressionIdx = -1; }
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   struct ProgressionChord
   {
      ProgressionChord(int degree, int beatLength = -1) : mDegree(degree), mBeatLength(beatLength), mInversion(0) {}
      
      ProgressionChord(int degree, std::vector<Accidental> accidentals, int beatLength = -1) : mDegree(degree), mAccidentals(accidentals), mBeatLength(beatLength), mInversion(0) {}
      
      ProgressionChord(const ofxJSONElement& chordInfo, ScalePitches scale);
      
      bool SameChord(const ProgressionChord& chord)
      {
         return mDegree == chord.mDegree &&
                mAccidentals == chord.mAccidentals;
      }
      int mDegree;
      int mBeatLength;
      std::vector<Accidental> mAccidentals;
      int mInversion;
   };
   
   struct SongSection
   {
      string mName;
      std::vector<ProgressionChord> mChords;
   };
   
   struct Song
   {
      string mName;
      float mTempo;
      int mTimeSigTop;
      int mTimeSigBottom;
      int mScaleRoot;
      string mScaleType;
      std::vector<SongSection> mSections;
   };
   
   void SetPitchColor(int pitch);
   void DrawKeyboard(float x, float y);
   void DrawGuitar(float x, float y);
   bool IsChordRoot(int pitch);
   void ReadSongs();
   void UpdateProgression(int beat);
   void GenerateRandomProgression();
   vector<int> GetCurrentChordPitches();
   ofRectangle GetKeyboardKeyRect(int pitch, bool& isBlackKey);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=610; height=700; }
   void OnClicked(int x, int y, bool right) override;
   
   ClickButton* mChaosButton;
   bool mTotalChaos;
   Checkbox* mTotalChaosCheckbox;
   double mChaosArrivalTime;
   int mLastAudioUpdateMeasure;
   int mLastAudioUpdateBeat;
   int mBeatsLeftToChordChange;
   bool mRestarting;
   
   int mChordProgressionIdx;
   std::vector<ProgressionChord> mChordProgression;
   ofMutex mProgressionMutex;
   IntSlider* mChordProgressionSlider;
   
   int mSectionIdx;
   RadioButton* mSectionDropdown;
   
   int mSongIdx;
   DropdownList* mSongDropdown;
   std::vector<Song> mSongs;
   ClickButton* mReadSongsButton;
   
   bool mPlayChord;
   Checkbox* mPlayChordCheckbox;
   bool mProgress;
   Checkbox* mProgressCheckbox;
   
   int mDegree;
   IntSlider* mDegreeSlider;
   Chord mInputChord;
   DropdownList* mRootNoteList;
   DropdownList* mChordTypeList;
   IntSlider* mInversionSlider;
   
   vector<Chord> mInputChords;
   ClickButton* mAddChordButton;
   ClickButton* mRemoveChordButton;
   ClickButton* mSetProgressionButton;
   
   ClickButton* mRandomProgressionButton;
   
   bool mHideBeat;
   Checkbox* mHideBeatCheckbox;
};

#endif /* defined(__modularSynth__ChaosEngine__) */

